//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_HANDLE_H
#define LIBUVCXX_HANDLE_H

#include <cassert>

#include "cxx/callback.h"
#include "cxx/promise.h"
#include "inner/base.h"

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
    class handle_t : public uvcxx::shared_raw_base_t<uv_handle_t> {
    public:
        using self = handle_t;
        using supper = uvcxx::shared_raw_base_t<uv_handle_t>;

        using supper::supper;

        [[nodiscard]]
        loop_t loop() const {
            return loop_t::borrow(raw<raw_t>()->loop);
        }

        [[nodiscard]]
        uv_handle_type type() const {
            return raw<raw_t>()->type;
        }

        [[nodiscard]]
        void *data() const {
            return raw()->data;
        }

        [[nodiscard]]
        bool is_active() const {
            return uv_is_active(*this);
        }

        [[nodiscard]]
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

        [[nodiscard]]
        size_t size() const {
            return uv_handle_size(raw()->type);
        }

        [[nodiscard]]
        loop_t get_loop() const {
            return loop();
        }

        [[nodiscard]]
        uv_handle_type get_type() const {
            return raw()->type;
        }

#if UVCXX_SATISFY_VERSION(1, 19, 0)

        [[nodiscard]]
        const char *type_name() const {
            return uv_handle_type_name(raw()->type);
        }

#endif

        void close(std::nullptr_t) {
            close_for([&](void (*cb)(raw_t *)) {
                uv_close(*this, cb);
            });
        }

        [[nodiscard("use close(nullptr) instead")]]
        uvcxx::promise<> close() {
            return close_for_promise([&](void (*cb)(raw_t *)) {
                uv_close(*this, cb);
            });
        }

        [[nodiscard]]
        void *get_data() const {
            return raw()->data;
        }

        template<typename T>
        [[nodiscard]]
        T *data() const {
            return (T *) raw()->data;
        }

        template<typename T>
        [[nodiscard]]
        T *get_data() const {
            return (T *) raw()->data;;
        }

        /**
         * DO NOT this method on your own as it may result in unexpected failure.
         * @param data
         */
        void set_data(void *data) {
            raw()->data = data;
        }

    protected:
        void close_for(const std::function<void(void (*)(raw_t *))> &close) const {
            auto data = get_data<data_t>();
            if (!data_t::is_it(data)) {
                throw uvcxx::errcode(UV_EPERM, "close non-libuvcxx handle is not permitted");
            }

            // Ensure that the handle will only be closed once, avoiding multiple invocations of close
            //     due to the use of asynchronous queues in the callback's `queue` working mode.
            bool closed = false;
            if (!data->closed.compare_exchange_strong(closed, true)) return;

            close(raw_close_callback);
        }

        uvcxx::promise<> close_for_promise(const std::function<void(void (*)(raw_t *))> &close) const {
            auto data = get_data<data_t>();
            if (!data_t::is_it(data)) {
                throw uvcxx::errcode(UV_EPERM, "close non-libuvcxx handle is not permitted");
            }

            // Ensure that the handle will only be closed once, avoiding multiple invocations of close
            //     due to the use of asynchronous queues in the callback's `queue` working mode.
            bool closed = false;
            if (!data->closed.compare_exchange_strong(closed, true)) return {};

            data->close_cb = decltype(data->close_cb)();
            close(raw_close_callback);

            return data->close_cb.promise();
        }

    public:
        /**
         * the uv_handle_t->data must be the sub-class of data_t
         */
        class data_t {
        public:
            static constexpr uint64_t MAGIC = 0x1155665044332210;
            uint64_t magic = MAGIC;

            std::atomic<bool> closed{false};
            uvcxx::promise_emitter<> close_cb = nullptr;

            static bool is_it(void *data) {
                return data && ((data_t *) data)->magic == MAGIC;
            }

        public:
            explicit data_t(const handle_t &handle)
                    : m_handle(handle.shared_raw()) {
                if (m_handle->data) throw uvcxx::errcode(UV_EPERM, "duplicated initialization of data");
                m_handle->data = this;
            }

            virtual ~data_t() {
                m_handle->data = nullptr;
            };

            virtual void close() noexcept {};

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

    protected:
        static self borrow(raw_t *raw) {
            return self{borrow_t(raw)};
        }

    private:
        static void raw_close_callback(raw_t *raw) {
            auto data = (data_t *) raw->data;
            if (!data) return;
            uvcxx::defer delete_data(std::default_delete<data_t>(), data);
            // reset data to nullptr, avoid accidentally using this field.
            uvcxx::defer reset_data([&]() { raw->data = nullptr; });
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

    template<typename T, typename B, typename=typename std::enable_if_t<std::is_base_of_v<handle_t, B>>>
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
            return std::reinterpret_pointer_cast<uv_handle_t>(std::make_shared<T>());
        }
    };
}

namespace uvcxx {
    /**
     * Keep the reference of the `handle` and close it when there are no references to it.
     * No relationship to `uv_ref` and `uv_unref`.
     * Usage:
     * ```
     * {
     *     uvcxx::ref idle = uv::idle_t();
     *     // After going out of scope, `idle` will automatically call close.
     * }
     * ```
     * The `ref` can be copied, and when it is no longer referenced, `close` will be called.
     * @tparam Handle
     */
    template<typename Handle, typename Enable = void>
    class ref;

    template<typename Handle>
    class ref<Handle, typename std::enable_if_t<
            std::is_copy_constructible_v<Handle> &&
            std::is_base_of_v<uv::handle_t, Handle>>> {
    public:
        using self = ref;

        /**
         * Create an object without an actual referenced instance.
         * You can use bool(ref) to determine if the reference is actually held.
         */
        ref(std::nullptr_t) {}

        /**
         * Create an object with a referenced instance handle.
         * @param handle
         */
        ref(Handle handle) : m_handle(make_shared(std::move(handle))) {}

        /**
         * Create an object with a referenced instance handle.
         * @param handle
         */
        ref(const std::shared_ptr<Handle> &handle) : m_handle(make_shared(*handle)) {}

        /**
         * Remove the data associated with the current reference.
         * If the reference is cleared, close will be automatically called.
         * After this function is called, the object can no longer be operated on.
         * Generally, there is no need to explicitly perform this operation.
         * You can use bool(ref) to determine if the reference is actually held.
         */
        void unref() {
            m_handle.reset();
        }

        explicit operator bool() const { return m_handle; }

        Handle &operator*() { return *m_handle; }

        const Handle &operator*() const { return *m_handle; }

        Handle *operator->() { return m_handle.get(); }

        const Handle *operator->() const { return m_handle.get(); }

    private:
        std::shared_ptr<Handle> m_handle;

        static std::shared_ptr<Handle> make_shared(Handle handle) {
            return std::shared_ptr<Handle>(new Handle(std::move(handle)), [](Handle *handle) {
                handle->close(nullptr);
                delete handle;
            });
        }
    };

    template<typename Handle>
    class ref<Handle, typename std::enable_if_t<
            !std::is_copy_constructible_v<Handle> &&
            std::is_base_of_v<uv::handle_t, Handle>>> {
    public:
        using self = ref;

        using PHandle = std::shared_ptr<Handle>;

        /**
         * Create an object without an actual referenced instance.
         * You can use bool(ref) to determine if the reference is actually held.
         */
        ref(std::nullptr_t) {}

        /**
         * Create an object with a referenced instance handle.
         * @param handle
         */
        ref(const std::shared_ptr<Handle> &handle) : m_handle(make_shared(handle)) {}

        /**
         * Remove the data associated with the current reference.
         * If the reference is cleared, close will be automatically called.
         * After this function is called, the object can no longer be operated on.
         * Generally, there is no need to explicitly perform this operation.
         * You can use bool(ref) to determine if the reference is actually held.
         */
        void unref() {
            m_handle.reset();
        }

        explicit operator bool() const { return m_handle; }

        Handle &operator*() { return **m_handle; }

        const Handle &operator*() const { return **m_handle; }

        Handle *operator->() { return (*m_handle).get(); }

        const Handle *operator->() const { return (*m_handle).get(); }

    private:
        std::shared_ptr<PHandle> m_handle;

        static std::shared_ptr<PHandle> make_shared(PHandle handle) {
            return std::shared_ptr<PHandle>(new PHandle(std::move(handle)), [](PHandle *handle) {
                (*handle)->close(nullptr);
                delete handle;
            });
        }
    };

    template<typename Handle>
    ref(Handle handle) -> ref<Handle>;

    template<typename Handle>
    ref(const std::shared_ptr<Handle> &handle) -> ref<Handle>;
}

#endif //LIBUVCXX_HANDLE_H
