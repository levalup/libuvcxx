//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_HANDLE_H
#define LIBUVCXX_HANDLE_H

#include <cassert>

#include "inner/base.h"
#include "utils/callback.h"
#include "utils/promise.h"
#include "utils/detach.h"

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
     * The data field of uv_handle_t will be retained and used by the UVCXX.
     * Please do not modify data.
     * The data' resources will be freed after close. So remember to `close` handle.
     * In most of case, only the handles in `Running` state need to be `close` manually.
     * See [lifecycle.md](https://github.com/levalup/libuvcxx/blob/master/docs/lifecycle.md) for more details.
     * Q: Why not `close` in destructor?
     * A: Because the destructor only be called after it is closed.
     */
    class handle_t : public uvcxx::attach_t, public uvcxx::shared_raw_base_t<uv_handle_t> {
    public:
        using self = handle_t;
        using supper = uvcxx::shared_raw_base_t<uv_handle_t>;

        using supper::supper;

        UVCXX_NODISCARD
        loop_t loop() const {
            return loop_t::borrow(raw<raw_t>()->loop);
        }

        UVCXX_NODISCARD
        uv_handle_type type() const {
            return raw<raw_t>()->type;
        }

        UVCXX_NODISCARD
        void *data() const {
            return raw()->data;
        }

        UVCXX_NODISCARD
        bool is_active() const {
            return uv_is_active(*this);
        }

        UVCXX_NODISCARD
        bool is_closing() const {
            return uv_is_closing(*this);
        }

        void ref() {
            uv_ref(*this);
        }

        void unref() {
            uv_unref(*this);
        }

        bool has_ref() {
            return uv_has_ref(*this);
        }

        UVCXX_NODISCARD
        size_t size() const {
            return uv_handle_size(raw()->type);
        }

        UVCXX_NODISCARD
        loop_t get_loop() const {
            // cover uv_handle_get_loop
            return loop();
        }

        UVCXX_NODISCARD
        uv_handle_type get_type() const {
            // cover uv_handle_get_type
            return raw()->type;
        }

#if UVCXX_SATISFY_VERSION(1, 19, 0)

        UVCXX_NODISCARD
        const char *type_name() const {
            return uv_handle_type_name(raw()->type);
        }

#endif

        void close(std::nullptr_t) {
            (void) close_for([&](void (*cb)(raw_t *)) {
                uv_close(*this, cb);
            });
        }

        UVCXX_NODISCARD
        uvcxx::promise<> close() {
            return close_for([&](void (*cb)(raw_t *)) {
                uv_close(*this, cb);
            });
        }

        UVCXX_NODISCARD
        void *get_data() const {
            // cover uv_handle_get_data
            return raw()->data;
        }

        template<typename T>
        UVCXX_NODISCARD
        T *data() const {
            return (T *) raw()->data;
        }

        template<typename T>
        UVCXX_NODISCARD
        T *get_data() const {
            return (T *) raw()->data;;
        }

        /**
         * DO NOT this method on your own as it may result in unexpected failure.
         * @param data
         */
        void set_data(void *data) {
            // cover uv_handle_set_data
            raw()->data = data;
        }

    protected:
        uvcxx::promise<> close_for(const std::function<void(void (*)(raw_t *))> &close) {
            auto data = get_data<data_t>();
            if (!data_t::is_it(data)) {
                throw uvcxx::errcode(UV_EPERM, "close invalid libuvcxx handle");
            }

            (void) data->close_for(close);
            _detach_();

            return data->close_cb.promise();
        }

        void _attach_data_() {
            _attach_(this->attach_data());
        }

        void _attach_close_() {
            _attach_(this->attach_close());
        }

    private:
        std::function<void(void)> attach_data() {
            auto handle = raw();
            return [handle]() {
                auto data = (data_t *) handle->data;
                if (!data_t::is_it(data)) return;
                data->close_for([&](void (*cb)(raw_t *)) {
                    // directly delete data and emit close promise
                    cb(handle);
                });
            };
        }

        std::function<void(void)> attach_close() {
            auto handle = raw();
            return [handle]() {
                auto data = (data_t *) handle->data;
                if (!data_t::is_it(data)) return;

                data->close_for([&](void (*cb)(raw_t *)) {
                    uv_close(handle, cb);
                });
            };
        }

    public:
        /**
         * the uv_handle_t->data must be the sub-class of data_t
         */
        class data_t {
        public:
            static constexpr uint64_t MAGIC = 0x1155665044332210;
            uint64_t magic = MAGIC;

            uvcxx::promise_emitter<> close_cb;

            static bool is_it(void *data) {
                return data && ((data_t *) data)->magic == MAGIC;
            }

        public:
            explicit data_t(const handle_t &handle)
                    : m_handle(handle.shared_raw()) {
                if (m_handle->data) throw uvcxx::errcode(UV_EPERM, "duplicated data initialization");
            }

            virtual ~data_t() = default;

            raw_t *handle() { return m_handle.get(); }

            UVCXX_NODISCARD
            const raw_t *handle() const { return m_handle.get(); }

            template<typename T>
            T *handle() { return (T *) m_handle.get(); }

            template<typename T>
            UVCXX_NODISCARD
            const T *handle() const { return (const T *) m_handle.get(); }

        public:
            /**
             * Ensure that the handle will only be closed once, avoiding multiple invocations of close
             *     due to the use of asynchronous queues in the callback's `queue` working mode.
             * @param close close cb, could be `uv_close` or `uv_tcp_close_reset`.
             * @return
             */
            bool close_for(const std::function<void(void (*)(raw_t *))> &close) {
                bool closed = false;
                if (!m_closed.compare_exchange_strong(closed, true)) return false;

                close(raw_close_callback);

                return true;
            }

        private:
            // store the instance of `handle` to avoid resource release caused by no external reference
            std::shared_ptr<raw_t> m_handle;

            std::atomic<bool> m_closed{false};
        };

    protected:
        static self borrow(raw_t *raw) {
            return self{borrow_t(raw)};
        }

    private:
        static void raw_close_callback(raw_t *raw) {
            auto data = (data_t *) raw->data;
            if (!data) return;

            uvcxx::defer delete_data(std::default_delete<data_t>(), data);
            uvcxx::defer reset_data([&]() { raw->data = nullptr; });

            data->close_cb.resolve();
        }

    protected:
        template<typename...ARGS>
        void watch(uvcxx::callback<ARGS...> &&callback) {
            auto handle = raw();
            callback.template except<uvcxx::close_handle>([handle]() mutable {
                auto data = (data_t *) handle->data;
                data->close_for([handle](void (*cb)(raw_t *)) {
                    uv_close(handle, cb);
                });
            });
        }

        template<typename...ARGS>
        void watch(const uvcxx::callback_emitter<ARGS...> &callback) {
            watch(callback.callback());
        }

        template<typename C, typename...ARGS>
        void watch(const uvcxx::callback_cast<C, ARGS...> &callback) {
            watch(callback.callback());
        }
    };

    template<typename T, typename B, typename=typename std::enable_if<std::is_base_of<handle_t, B>::value>::type>
    class inherit_handle_t : public B {
    public:
        using self = inherit_handle_t;
        using supper = B;
        using raw_t = T;

        using supper::supper;

        inherit_handle_t()
                : supper(make_shared()) {
            this->set_data(nullptr);
        }

        operator T *() { return this->template raw<T>(); }

        operator T *() const { return this->template raw<T>(); }

    private:
        static std::shared_ptr<uv_handle_t> make_shared() {
            auto r = std::make_shared<T>();
            auto p = reinterpret_cast<typename std::shared_ptr<uv_handle_t>::element_type*>(r.get());
            return std::shared_ptr<uv_handle_t>{r, p};
        }
    };
}

#endif //LIBUVCXX_HANDLE_H
