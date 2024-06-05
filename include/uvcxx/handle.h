//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_HANDLE_H
#define LIBUVCXX_HANDLE_H

#include <cassert>

#include "cxx/callback.h"
#include "cxx/promise.h"

#include "loop.h"

namespace uvcxx {
    class close_handle : public std::logic_error {
    public:
        using self = close_handle;
        using supper = std::logic_error;

        close_handle() : supper("close handle") {}
    };
}

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
        class data_t {
        public:
            std::atomic<bool> closed{false};
            uvcxx::promise_emitter<> close_cb = nullptr;

        public:
            explicit data_t(const handle_t &handle)
                    : m_handle(handle.m_raw) {
                m_handle->data = this;
            }

            virtual ~data_t() {
                m_handle->data = nullptr;
            };

            virtual void close() noexcept = 0;

            raw_t *handle() { return m_handle.get(); }

            [[nodiscard]]
            const raw_t *handle() const { return m_handle.get(); }

            template<typename T>
            T *handle() { return (T *) m_handle.get(); }

            template<typename T>
            [[nodiscard]]
            const T *handle() const { return (const T *) m_handle.get(); }

        private:
            // store the instance of `handle` to avoid resource release caused by no external reference
            std::shared_ptr<raw_t> m_handle;
        };

        [[nodiscard]]
        bool is_active() const {
            return uv_is_active(m_raw.get());
        }

        [[nodiscard]]
        bool is_closing() const {
            return uv_is_closing(m_raw.get());
        }

        void close(std::nullptr_t) {
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

        template<typename T>
        [[nodiscard]]
        T *get_data() const {
            return (T *) m_raw->data;
        }

        operator raw_t *() { return m_raw.get(); }

        operator raw_t *() const { return m_raw.get(); }

        explicit operator bool() { return bool(m_raw); }

        static self borrow(raw_t *raw) {
            return self{std::shared_ptr<raw_t>(raw, [](raw_t *) {})};
        }

    protected:
        explicit handle_t(std::shared_ptr<raw_t> raw)
                : m_raw(std::move(raw)) {}

        raw_t *raw() { return m_raw.get(); }

        raw_t *raw() const { return m_raw.get(); }

        template<typename T>
        T *raw() { return (T *) (m_raw.get()); }

        template<typename T>
        T *raw() const { return (T *) (m_raw.get()); }

    private:
        std::shared_ptr<raw_t> m_raw;

    private:
        static void raw_close_callback(raw_t *raw) {
            auto data = (data_t *) raw->data;
            if (!data) return;
            uvcxx::defer delete_data(std::default_delete<data_t>(), data);
            uvcxx::defer close_data([&]() { data->close(); });

            if (data->close_cb) data->close_cb.resolve();
        }

    protected:
        template<typename...ARGS>
        void watch(uvcxx::callback<ARGS...> &callback) {
            callback.template except<uvcxx::close_handle>([*this]() mutable {
                close(nullptr);
            });
        }

        template<typename...ARGS>
        void watch(const uvcxx::callback_emitter<ARGS...> &callback) {
            callback.callback().template except<uvcxx::close_handle>([*this]() mutable {
                close(nullptr);
            });
        }

        template<typename C, typename...ARGS>
        void watch(const uvcxx::callback_cast<C, ARGS...> &callback) {
            callback.callback().template except<uvcxx::close_handle>([*this]() mutable {
                close(nullptr);
            });
        }
    };

    class handle_fd_t : virtual public handle_t {
    public:
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

        operator T *() { return this->template raw<T>(); }

        operator T *() const { return this->template raw<T>(); }

    protected:
        explicit handle_extend_t(const std::shared_ptr<T> &raw)
                : supper(std::reinterpret_pointer_cast<uv_handle_t>(raw)) {}

    private:
        static std::shared_ptr<uv_handle_t> make_shared() {
            return std::reinterpret_pointer_cast<uv_handle_t>(std::make_shared<T>());
        }
    };
}

#endif //LIBUVCXX_HANDLE_H
