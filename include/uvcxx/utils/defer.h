//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_DEFER_H
#define LIBUVCXX_DEFER_H

#include <functional>
#include <type_traits>

#include "standard.h"

namespace uvcxx {
    template<typename T>
    auto decay_val() -> typename std::decay<T>::type;

    template<typename FUNC, typename...Args>
    struct invoke_result {
        using type = decltype(std::bind(
                decay_val<FUNC>(),
                decay_val<Args>()...)());
    };

    template<typename... Args>
    struct is_invocable_impl_args {
        template<typename FUNC, typename = typename invoke_result<FUNC, Args...>::type>
        static auto check(int) -> std::true_type { return {}; }

        template<typename ...>
        static auto check(...) -> std::false_type { return {}; }
    };

    template<typename FUNC, typename...Args>
    struct is_invocable_impl {
    public:
        using type = decltype(is_invocable_impl_args<Args...>::template check<FUNC>(0));
    };

    template<typename FUNC, typename... Args>
    struct is_invocable : public is_invocable_impl<FUNC, Args...>::type {
    };

    class defer {
    public:
        using self = defer;

        defer(const defer &) = delete;

        defer &operator=(const defer &) = delete;

        template<typename FUNC, typename... Args,
                typename std::enable_if<is_invocable<FUNC, Args...>::value, int>::type = 0>
        explicit defer(FUNC finalizer, Args &&... args)
                : m_finalizer(self::bind(finalizer, std::forward<Args>(args)...)) {
        }

        ~defer() {
            if (m_finalizer) m_finalizer();
        }

        defer(defer &&that) UVCXX_NOEXCEPT { std::swap(m_finalizer, that.m_finalizer); }

        defer &operator=(defer &&that) UVCXX_NOEXCEPT {
            std::swap(m_finalizer, that.m_finalizer);
            return *this;
        }

        /**
         * Release the needed finalizer without calling it.
         * :note danger action, do not use this.
         */
        void release() { m_finalizer = nullptr; }

        /**
         * Call the needed finalizer immediately.
         */
        void emit() {
            if (m_finalizer) m_finalizer();
            m_finalizer = nullptr;
        }

    protected:
        template<typename FUNC, typename... Args, typename std::enable_if<
                std::is_same<void, typename invoke_result<FUNC, Args...>::type>::value, int>::type = 0>
        static std::function<void()> bind(FUNC func, Args &&... args) {
            return std::bind(func, std::forward<Args>(args)...);
        }

        template<typename FUNC, typename... Args, typename std::enable_if<
                !std::is_same<void, typename invoke_result<FUNC, Args...>::type>::value &&
                std::is_constructible<std::function<void()>,
                        decltype(std::bind(std::declval<FUNC>(), std::declval<Args>()...))>::value, int>::type = 0>
        static std::function<void()> bind(FUNC func, Args &&... args) {
            return std::bind(func, std::forward<Args>(args)...);
        }

        template<typename FUNC, typename... Args, typename std::enable_if<
                !std::is_same<void, typename invoke_result<FUNC, Args...>::type>::value &&
                !std::is_constructible<std::function<void()>,
                        decltype(std::bind(std::declval<FUNC>(), std::declval<Args>()...))>::value, int>::type = 0>
        static std::function<void()> bind(FUNC func, Args &&... args) {
// Initialized/Generalized lambda captures (init-capture)
#if __cpp_init_captures >= 201304L || __cplusplus >= 201402L || _MSC_VER >= 1900
            return [void_call = std::bind(func, std::forward<Args>(args)...)]() -> void {
                (void) void_call();
            };
#else
            auto void_call = std::bind(func, std::forward<Args>(args)...);
            return [void_call]() -> void { (void) void_call(); };
#endif
        }

    private:
        std::function<void()> m_finalizer;
    };

    template<typename T>
    class defer_delete {
    public:
        using self = defer_delete;

        explicit defer_delete(T *p)
                : m_defer(std::default_delete<T>(), p) {}

        defer_delete(const defer_delete &) = delete;

        defer_delete &operator=(const defer_delete &) = delete;

        defer_delete(defer_delete &&that) UVCXX_NOEXCEPT {
            (void) this->operator=(std::move(that));
        }

        defer_delete &operator=(defer_delete &&that) UVCXX_NOEXCEPT {
            std::swap(m_defer, that.m_defer);
            return *this;
        }

        void release() { m_defer.release(); }

        void emit() { m_defer.emit(); }

    private:
        defer m_defer;
    };

    template<typename T>
    class defer_delete<T[]> {
    public:
        using self = defer_delete;

        explicit defer_delete(T *p)
                : m_defer(std::default_delete<T[]>(), p) {}

        defer_delete(const defer_delete &) = delete;

        defer_delete &operator=(const defer_delete &) = delete;

        defer_delete(defer_delete &&that) UVCXX_NOEXCEPT {
            (void) this->operator=(std::move(that));
        }

        defer_delete &operator=(defer_delete &&that) UVCXX_NOEXCEPT {
            std::swap(m_defer, that.m_defer);
            return *this;
        }

        void release() { m_defer.release(); }

        void emit() { m_defer.emit(); }

    private:
        defer m_defer;
    };
// Class template argument deduction
#if __cpp_deduction_guides >= 201703L || __cplusplus >= 201703L || _MSC_VER >= 1914
    template<typename T>
    defer_delete(T *p) -> defer_delete<std::decay_t<decltype(*p)>>;
#endif
}

#endif // LIBUVCXX_DEFER_H
