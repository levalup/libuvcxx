//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_HANDLE_H
#define LIBUVCXX_HANDLE_H

#include <memory>
#include <functional>
#include <cassert>

#include <uv.h>

#include "cxx/defer.h"
#include "cxx/except.h"
#include "cxx/promise.h"

#include "loop.h"

namespace uv {
    /**
     * The data field of uv_req_t will be retained and used by the UVCXX.
     * Please do not modify data.
     * The data' resources will be freed after close. So remember to `close` handle.
     * Do `close` even if not start but only construct it.
     * Q: Why not `close` in destructor?
     * A: Because the destructor only be called after it is closed.
     */
    class handle_t {
    public:
        using self = handle_t;
        using raw_t = uv_handle_t;

        /**
         * the uv_handle_t->data must be the sub-class of data_t
         */
        struct data_t {
            virtual ~data_t() = default;

            std::atomic<bool> closed{false};
            uvcxx::promise_emitter<> close_cb = nullptr;

            virtual void close() noexcept = 0;
        };

        [[nodiscard]]
        bool is_active() const {
            return uv_is_active(m_raw.get());
        }

        [[nodiscard]]
        bool is_closing() const {
            return uv_is_closing(m_raw.get());
        }

        void close(nullptr_t) {
            assert(m_raw->data != nullptr);
            auto data = (data_t *) m_raw->data;

            // Ensure that the handle will only be closed once, avoiding multiple invocations of close
            //     due to the use of asynchronous queues in the callback's `queue` working mode.
            bool closed = false;
            if (!data->closed.compare_exchange_strong(closed, true)) return;

            uv_close(m_raw.get(), raw_close_callback);
        }

        [[nodiscard("use close(nullptr) instead")]]
        uvcxx::promise<> close() {
            assert(m_raw->data != nullptr);
            auto data = (data_t *) m_raw->data;

            // Ensure that the handle will only be closed once, avoiding multiple invocations of close
            //     due to the use of asynchronous queues in the callback's `queue` working mode.
            bool closed = false;
            if (!data->closed.compare_exchange_strong(closed, true)) return {};

            data->close_cb = decltype(data->close_cb)();
            uv_close(m_raw.get(), raw_close_callback);

            return data->close_cb.promise();
        }

        void ref() {
            uv_ref(m_raw.get());
        }

        void unref() {
            uv_unref(m_raw.get());
        }

        bool has_ref() {
            return uv_has_ref(m_raw.get());
        }

        [[nodiscard]]
        size_t size() const {
            return uv_handle_size(m_raw->type);
        }

        [[nodiscard]]
        loop_t loop() const {
            return loop_t::borrow(uv_handle_get_loop(m_raw.get()));
        }

        [[nodiscard]]
        void *get_data() const {
            return uv_handle_get_data(m_raw.get());
        }

        /**
         * Never use this method on your own as it may result in failure.
         * @param data
         */
        void set_data(void *data) {
            uv_handle_set_data(m_raw.get(), data);
        }

        [[nodiscard]]
        uv_handle_type get_type() const {
            return uv_handle_get_type(m_raw.get());
        }

        [[nodiscard]]
        const char *type_name() const {
            return uv_handle_type_name(m_raw->type);
        }

        operator raw_t *() const { return m_raw.get(); }

        operator const raw_t *() const { return m_raw.get(); }

    protected:
        explicit handle_t(std::shared_ptr<raw_t> raw)
                : m_raw(std::move(raw)) {}

        raw_t *raw() { return m_raw.get(); }

        const raw_t *raw() const { return m_raw.get(); }

        template<typename T>
        T *raw() { return reinterpret_cast<T *>(m_raw.get()); }

        template<typename T>
        const T *raw() const { return reinterpret_cast<T *>(m_raw.get()); }

    private:
        std::shared_ptr<raw_t> m_raw;

    private:
        static void raw_close_callback(raw_t *raw) {
            auto data = (data_t *) raw->data;
            if (!data) return;
            uvcxx::defer delete_data(std::default_delete<data_t>(), data);
            uvcxx::defer reset_data([&]() { raw->data = nullptr; });
            uvcxx::defer close_data([&]() { data->close(); });

            if (data->close_cb) data->close_cb.resolve();
        }
    };

    class handle_buffer_t : public handle_t {
    public:
        using self = handle_buffer_t;
        using supper = handle_t;

        using supper::supper;

        int send_buffer_size(int *value) {
            auto err = uv_send_buffer_size(raw(), value);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int recv_buffer_size(int *value) {
            auto err = uv_recv_buffer_size(raw(), value);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }
    };

    class handle_fd_t : public handle_t {
    public:
        using self = handle_buffer_t;
        using supper = handle_t;

        using supper::supper;

        int fileno(uv_os_fd_t *fd) const {
            auto err = uv_fileno(raw(), fd);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }
    };

    template<typename T, typename B, typename=typename std::enable_if_t<std::is_base_of_v<handle_t, B>>>
    class handle_extend_t : public B {
    public:
        using self = handle_extend_t;
        using supper = B;
        using raw_t = T;

        handle_extend_t()
                : supper(make_shared()) {
            this->set_data(nullptr);
        }

        operator T *() { return (T *) this->raw(); }

        operator const T *() const { return (const T *) this->raw(); }

    protected:
        explicit handle_extend_t(const std::shared_ptr<T> &raw)
                : supper(std::reinterpret_pointer_cast<supper::raw_t>(raw)) {}

    private:
        static std::shared_ptr<typename supper::raw_t> make_shared() {
            return std::reinterpret_pointer_cast<typename supper::raw_t>(std::make_shared<T>());
        }
    };
}

#endif //LIBUVCXX_HANDLE_H
