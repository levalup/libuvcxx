//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CXX_REF_H
#define LIBUVCXX_CXX_REF_H

#include "../handle.h"

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
    class ref<Handle, typename std::enable_if<
            std::is_copy_constructible<Handle>::value &&
            std::is_base_of<uv::handle_t, Handle>::value>::type> {
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
    class ref<Handle, typename std::enable_if<
            !std::is_copy_constructible<Handle>::value &&
            std::is_base_of<uv::handle_t, Handle>::value>::type> {
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
// Class template argument deduction
#if __cpp_deduction_guides >= 201703L || __cplusplus >= 201703L || _MSC_VER >= 1914
    template<typename Handle>
    ref(Handle handle) -> ref<Handle>;

    template<typename Handle>
    ref(const std::shared_ptr<Handle> &handle) -> ref<Handle>;
#endif
}

#endif //LIBUVCXX_CXX_REF_H
