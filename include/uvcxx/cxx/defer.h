//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_DEFER_H
#define LIBUVCXX_DEFER_H

#include <type_traits>
#include <functional>

namespace uvcxx {
    class defer {
    public:
        using self = defer;

        defer(const defer &) = delete;

        defer &operator=(const defer &) = delete;

        template<typename FUNC, typename... Args,
                typename = typename std::enable_if_t<std::is_invocable_v<FUNC, Args...>>>
        explicit defer(FUNC finalizer, Args &&... args)
            : m_finalizer(self::bind(finalizer, std::forward<Args>(args)...)) {
        }

        ~defer() {
            if (m_finalizer) m_finalizer();
        }

        defer(defer &&that) noexcept { std::swap(m_finalizer, that.m_finalizer); }

        defer &operator=(defer &&that) noexcept {
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
        template<typename FUNC, typename... Args>
        static
        typename std::enable_if_t<std::is_invocable_r_v<void, FUNC, Args...>, std::function<void()>>
        bind(FUNC func, Args &&... args) {
            return std::bind(func, std::forward<Args>(args)...);
        }

        template<typename FUNC, typename... Args>
        static
        typename std::enable_if_t<!std::is_invocable_r_v<void, FUNC, Args...>, std::function<void()>>
        bind(FUNC func, Args &&... args) {
            return [void_call = std::bind(func, std::forward<Args>(args)...)]() -> void {
                (void)void_call();
            };
        }

    private:
        std::function<void()> m_finalizer;
    };
}

#endif // LIBUVCXX_DEFER_H
