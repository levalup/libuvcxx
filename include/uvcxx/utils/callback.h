//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CALLBACK_H
#define LIBUVCXX_CALLBACK_H

#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <utility>

#include "apply.h"
#include "queue.h"
#include "standard.h"

namespace uvcxx {
    class callback_core {
    public:
        using self = callback_core;

        using on_call_t = std::function<void(void *)>;
        // Return whether this exception has been handled
        using on_except_t = std::function<bool(std::exception_ptr)>;
        using on_finally_t = std::function<void()>;

        ~callback_core() {
            finalize();
        }

        void emit(void *p) UVCXX_NOEXCEPT {
            try {
                if (m_on_call) m_on_call(p);
            } catch (...) {
                raise(std::current_exception());
            }
        }

        void raise(std::exception_ptr p) UVCXX_NOEXCEPT {
            try {
                if (m_on_except) (void) m_on_except(p);
                else default_on_except(p);
            } catch (...) {
                default_on_except(p);;
            }
        }

        void finalize() UVCXX_NOEXCEPT {
            bool finalized = false;
            if (!m_finalized.compare_exchange_strong(finalized, true)) return;
            try {
                if (m_on_finally) m_on_finally();
            } catch (...) {
            }
        }

        self &call(std::nullptr_t) {
            m_on_call = nullptr;
            return *this;
        }

        self &call(on_call_t f) {
            if (!f) return *this;
            if (m_on_call) {
// Initialized/Generalized lambda captures (init-capture)
#if __cpp_init_captures >= 201304L || __cplusplus >= 201402L || _MSC_VER >= 1900
                m_on_call = [f = std::move(f), pre = std::move(m_on_call)](void *p) {
                    pre(p);
                    f(p);
                };
#else
                auto pre = std::move(m_on_call);
                m_on_call = [f, pre](void *p) {
                    pre(p);
                    f(p);
                };
#endif
            } else {
                m_on_call = std::move(f);
            }
            return *this;
        }

        self &except(std::nullptr_t) {
            m_on_except = nullptr;
            return *this;
        }

        self &except(on_except_t f) {
            if (!f) return *this;
            if (m_on_except) {
// Initialized/Generalized lambda captures (init-capture)
#if __cpp_init_captures >= 201304L || __cplusplus >= 201402L || _MSC_VER >= 1900
                m_on_except = [f = std::move(f), pre = std::move(m_on_except)](std::exception_ptr p) {
                    if (pre(p)) return true;
                    return f(p);
                };
#else
                auto pre = std::move(m_on_except);
                m_on_except = [f, pre](std::exception_ptr p) {
                    if (pre(p)) return true;
                    return f(p);
                };
#endif
            } else {
                m_on_except = std::move(f);
            }
            return *this;
        }

        self &finally(std::nullptr_t) {
            m_on_finally = nullptr;
            return *this;
        }

        self &finally(on_finally_t f) {
            if (!f) return *this;
            if (m_on_finally) {
// Initialized/Generalized lambda captures (init-capture)
#if __cpp_init_captures >= 201304L || __cplusplus >= 201402L || _MSC_VER >= 1900
                m_on_finally = [f = std::move(f), pre = std::move(m_on_finally)]() {
                    pre();
                    f();
                };
#else
                auto pre = std::move(m_on_finally);
                m_on_finally = [f, pre]() {
                    pre();
                    f();
                };
#endif
            } else {
                m_on_finally = std::move(f);
            }
            return *this;
        }

        static void default_on_except(const std::exception &e) {
            std::ostringstream oss;
            oss << "[ERROR] " << e.what() << std::endl;
            std::cerr << oss.str();
        }

        static void default_on_except(std::exception_ptr p) {
            try {
                std::rethrow_exception(p);
            } catch (const std::exception &e) {
                default_on_except(e);
            } catch (...) {
                std::ostringstream oss;
                oss << "[ERROR] throw non - std::exception" << std::endl;
                std::cerr << oss.str();
            }
        }

    private:
        std::atomic<bool> m_finalized{false};
        on_call_t m_on_call;
        on_except_t m_on_except;
        on_finally_t m_on_finally;
    };

    /**
     * Set callback in return value.
     * @tparam T callback arguments
     * @note using `get_queue` will overwrite
     *     the callback functions of `call`, `raise` and `finally`.
     *     DO NOT use then in same time.
     * @note Call `call`, `raise`, `finally`, `get_queue` before run or while callback.
     */
    template<typename... T>
    class callback_t {
    public:
        using self = callback_t;
        using type = std::tuple<T...>;

        using on_call_t = std::function<void(const T &...)>;
        using on_except_t = std::function<bool(std::exception_ptr)>;
        using on_finally_t = std::function<void()>;

        callback_t() : m_core(std::make_shared<callback_core>()) {}

        callback_t(std::nullptr_t) : m_core(nullptr) {}

        explicit operator bool() const { return bool(m_core); }

        self &call(std::nullptr_t) {
            m_core->call(nullptr);
            return *this;
        }

        self &call(std::function<void(const T &...)> f) {
            m_core->call([UVCXX_CAPTURE_MOVE(f)](void *p) {
                auto &pack = *(const type *) (p);
                proxy_apply(f, pack);
            });
            return *this;
        }

        self &except(std::nullptr_t) {
            m_core->except(nullptr);
            return *this;
        }

        self &except(std::function<bool(std::exception_ptr)> f) {
            m_core->except(std::move(f));
            return *this;
        }

        template<typename E, typename std::enable_if<
                std::is_base_of<std::exception, E>::value, int>::type = 0>
        self &except(std::function<void(const E &)> f) {
            m_core->except([UVCXX_CAPTURE_MOVE(f)](std::exception_ptr p) -> bool {
                try {
                    std::rethrow_exception(p);
                } catch (const E &e) {
                    f(e);
                    return true;
                } catch (...) {
                    return false;
                }
            });
            return *this;
        }

        self &except(std::function<void(const std::exception &)> f) {
            return this->except<std::exception>(std::move(f));
        }

        template<typename E, typename std::enable_if<
                std::is_base_of<std::exception, E>::value, int>::type = 0>
        self &except(std::function<void()> f) {
            return this->except<E>([UVCXX_CAPTURE_MOVE(f)](const E &) { f(); });
        }

        self &finally(std::nullptr_t) {
            m_core->finally(nullptr);
            return *this;
        }

        self &finally(std::function<void()> f) {
            m_core->finally(std::move(f));
            return *this;
        }

        template<typename FUNC, typename std::enable_if<
                std::is_constructible<on_call_t, FUNC>::value, int>::type = 0>
        self &call(FUNC f) {
            return this->call(on_call_t(f));
        }

        template<typename FUNC, typename std::enable_if<
                !std::is_constructible<on_call_t, FUNC>::value && std::is_same<
                        decltype(std::declval<FUNC>()(std::declval<const T &>()...)),
                        void>::value, int>::type = 0>
        self &call(FUNC f) {
            return this->call(on_call_t([UVCXX_CAPTURE_MOVE(f)](const T &...args) {
                f(args...);
            }));
        }

        template<typename FUNC, typename std::enable_if<
                sizeof...(T) != 0 && std::is_same<
                        decltype(std::declval<FUNC>()()),
                        void>::value, int>::type = 0>
        self &call(FUNC f) {
            return this->call(on_call_t([UVCXX_CAPTURE_MOVE(f)](void *) {
                f();
            }));
        }

        template<typename FUNC, typename std::enable_if<std::is_same<
                decltype(std::declval<FUNC>()(std::declval<std::exception_ptr>())),
                void>::value, int>::type = 0>
        self &except(FUNC f) {
            return this->except(on_except_t([UVCXX_CAPTURE_MOVE(f)](std::exception_ptr p) -> bool {
                f(p);
                return false;
            }));
        }

        template<typename E, typename FUNC, typename std::enable_if<std::is_same<
                decltype(std::declval<FUNC>()(std::declval<const E &>())),
                void>::value, int>::type = 0>
        self &except(FUNC f) {
            return this->except<E>(std::function<void(const E &)>(std::move(f)));
        }

        template<typename E, typename FUNC, typename std::enable_if<std::is_same<
                decltype(std::declval<FUNC>()()),
                void>::value, int>::type = 0>
        self &except(FUNC f) {
            return this->except<E>(std::function<void()>(std::move(f)));
        }

        template<typename FUNC, typename std::enable_if<std::is_same<
                decltype(std::declval<FUNC>()()),
                void>::value, int>::type = 0>
        self &finally(FUNC f) {
            return this->finally(on_finally_t(f));
        }

        queue<T...> get_queue() {
            queue<T...> q;
            this->call([q](const T &...v) mutable {
                q.push(v...);
            }).finally([q]() mutable {
                q.close();
            });
            return q;
        }

    private:
        std::shared_ptr<callback_core> m_core;

        explicit callback_t(decltype(m_core) core) : m_core(std::move(core)) {}

    private:
        template<typename... K>
        friend
        class callback_emitter;

        template<typename... K>
        friend
        class callback_proxy;
    };

    template<typename... T>
    class callback_proxy {
    public:
        using self = callback_proxy;
        using type = std::tuple<T...>;

        virtual ~callback_proxy() = default;

        virtual void emit(const T &...v) UVCXX_NOEXCEPT = 0;

        virtual void raise(std::exception_ptr p) UVCXX_NOEXCEPT = 0;

        virtual void finalize() UVCXX_NOEXCEPT = 0;

        template<typename E, typename... ARGS,
                typename std::enable_if<
                        std::is_base_of<std::exception, E>::value &&
                        std::is_constructible<E, ARGS...>::value, int>::type = 0>
        void raise(ARGS &&...args) UVCXX_NOEXCEPT {
            try { throw E(std::forward<ARGS>(args)...); }
            catch (...) { this->raise(std::current_exception()); }
        }

        void apply(const std::tuple<T...> &v) UVCXX_NOEXCEPT {
            proxy_apply([this](const T &...v) {
                this->emit(v...);
            }, v);
        }

        void operator()(const T &...v) {
            this->emit(v...);
        }
    };

    template<typename... T>
    class callback_emitter : public callback_proxy<T...> {
    public:
        using self = callback_emitter;
        using supper = callback_proxy<T...>;

        callback_emitter() : m_core(std::make_shared<callback_core>()) {}

        callback_emitter(std::nullptr_t) : m_core(nullptr) {}

        explicit callback_emitter(const callback_t<T...> &p)
                : m_core(p.m_core) {}

        explicit operator bool() const { return bool(m_core); }

        void emit(const T &...v) UVCXX_NOEXCEPT final {
            std::tuple<T...> pack = std::make_tuple(v...);
            m_core->emit(&pack);
        }

        using supper::raise;

        void raise() UVCXX_NOEXCEPT {
            this->raise(std::current_exception());
        }

        void raise(std::exception_ptr p) UVCXX_NOEXCEPT final {
            m_core->raise(p);
        }

        void finalize() UVCXX_NOEXCEPT final {
            m_core->finalize();
        }

        UVCXX_NODISCARD
        callback_t<T...> callback() const { return callback_t<T...>{m_core}; }

    private:
        std::shared_ptr<callback_core> m_core;
    };

    template<typename V, typename... T>
    class callback_cast;

    template<typename... V, typename... T>
    class callback_cast<callback_t<V...>, T...> : public callback_proxy<T...> {
    public:
        using self = callback_cast;
        using supper = callback_proxy<T...>;

        using value_t = std::tuple<V...>;
        using wrapper_t = std::function<value_t(const T &...)>;

        template<typename Tuple>
        struct first_element {
            using type = void;
        };

        template<typename First, typename ...Else>
        struct first_element<std::tuple<First, Else...>> {
            using type = First;
        };

        callback_cast(std::nullptr_t) : m_emitter(nullptr) {}

        callback_cast(const callback_t<V...> &p, wrapper_t wrapper)
                : m_emitter(p), m_wrapper(std::move(wrapper)) {}

        template<typename FUNC, typename std::enable_if<
                std::is_constructible<wrapper_t, FUNC>::value, int>::type = 0>
        callback_cast(const callback_t<V...> &p, FUNC wrapper)
                : self(p, wrapper_t(wrapper)) {}

        template<typename FUNC, typename std::enable_if<
                !std::is_constructible<wrapper_t, FUNC>::value &&
                sizeof...(V) == 1 && std::is_convertible<
                        decltype(std::declval<FUNC>()(std::declval<const T &>()...)),
                        typename first_element<value_t>::type>::value, int>::type = 0>
        callback_cast(const callback_t<V...> &p, FUNC wrapper)
                : self(p, wrapper_t([UVCXX_CAPTURE_MOVE(wrapper)](const T &...value) -> value_t {
            return std::make_tuple(wrapper(value...));
        })) {}

        template<typename FUNC, typename std::enable_if<
                !std::is_constructible<wrapper_t, FUNC>::value &&
                sizeof...(V) == 0 && std::is_same<
                        decltype(std::declval<FUNC>()(std::declval<const T &>()...)),
                        void>::value, int>::type = 0>
        callback_cast(const callback_t<V...> &p, FUNC wrapper)
                : self(p, wrapper_t([UVCXX_CAPTURE_MOVE(wrapper)](const T &...value) -> value_t {
            wrapper(value...);
            return std::make_tuple();
        })) {}

        template<typename FUNC, typename std::enable_if<
                std::is_constructible<self, const callback_t<V...> &, FUNC>::value, int>::type = 0>
        explicit callback_cast(FUNC wrapper)
                : self(callback_t<V...>(), wrapper_t(wrapper)) {}

        explicit operator bool() const { return bool(m_emitter); }

        void emit(const T &...v) UVCXX_NOEXCEPT final {
            try {
                auto tmp = m_wrapper(v...);
                m_emitter.apply(tmp);
            } catch (...) {
                self::raise(std::current_exception());
                return;
            }
        }

        using supper::raise;

        void raise() UVCXX_NOEXCEPT {
            this->raise(std::current_exception());
        }

        void raise(std::exception_ptr p) UVCXX_NOEXCEPT final {
            m_emitter.raise(p);
        }

        void finalize() UVCXX_NOEXCEPT final {
            m_emitter.finalize();
        }

        UVCXX_NODISCARD
        callback_t<V...> callback() const { return m_emitter.callback(); }

    private:
        callback_emitter<V...> m_emitter;
        wrapper_t m_wrapper;
    };

    template<typename...T>
    using callback = callback_t<T...>;
}

#endif //LIBUVCXX_CALLBACK_H
