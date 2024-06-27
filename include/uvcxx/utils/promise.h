//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_PROMISE_H
#define LIBUVCXX_PROMISE_H

#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <sstream>
#include <utility>

#include "apply.h"
#include "standard.h"

namespace uvcxx {
    class promise_core {
    public:
        using self = promise_core;

        using on_then_t = std::function<void(void *)>;
        // Return whether this exception has been handled
        using on_except_t = std::function<bool(std::exception_ptr)>;
        using on_finally_t = std::function<void()>;

        ~promise_core() {
            finalize();
        }

        void resolve(void *p) UVCXX_NOEXCEPT {
            try {
                if (m_on_then) m_on_then(p);
            } catch (...) {
                reject(std::current_exception());
            }
        }

        /**
         * Allow MAX_RETRY times throw exception while processing `on_except`.
         * @param p exception pointer
         */
        void reject(std::exception_ptr p) UVCXX_NOEXCEPT {
            constexpr int MAX_RETRY = 1;
            if (m_on_except) {
                int caught = 0;
                while (true) {
                    try {
                        m_on_except(p);
                        return;
                    } catch (...) {
                        p = std::current_exception();
                    }
                    if (++caught > MAX_RETRY) break;
                }
            }
            default_on_except(p);
        }

        void finalize() UVCXX_NOEXCEPT {
            bool finalized = false;
            if (!m_finalized.compare_exchange_strong(finalized, true)) return;
            try {
                if (m_on_finally) m_on_finally();
            } catch (...) {
            }
        }

        self &then(std::nullptr_t) {
            m_on_then = nullptr;
            return *this;
        }

        self &then(on_then_t f) {
            if (!f) return *this;
            if (m_on_then) {
#if UVCXX_STD_INIT_CAPTURES
                m_on_then = [f = std::move(f), pre = std::move(m_on_then)](void *p) {
                    pre(p);
                    f(p);
                };
#else
                auto pre = std::move(m_on_then);
                m_on_then = [f, pre](void *p) {
                    pre(p);
                    f(p);
                };
#endif
            } else {
                m_on_then = std::move(f);
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
#if UVCXX_STD_INIT_CAPTURES
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
#if UVCXX_STD_INIT_CAPTURES
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

        static inline void default_on_except(const std::exception &e) UVCXX_NOEXCEPT {
            std::ostringstream oss;
            oss << "[ERROR] " << e.what() << std::endl;
            std::cerr << oss.str();
        }

        static inline void default_on_except(std::exception_ptr p) UVCXX_NOEXCEPT {
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
        on_then_t m_on_then;
        on_except_t m_on_except;
        on_finally_t m_on_finally;
    };

    /**
     * Javascript style promise, mix C++ style promise like std::promise.
     * @tparam T promise arguments
     * @note using `get_future` will overwrite
     *     the callback functions of `then`, `except` and `finally`.
     *     DO NOT use then in same time.
     * @note Call `then`, `except`, `finally`, `get_future` before run or while callback.
     */
    template<typename... T>
    class promise_t {
    public:
        using self = promise_t;
        using type = std::tuple<T...>;

        using on_then_t = std::function<void(const T &...)>;
        using on_except_t = std::function<bool(std::exception_ptr)>;
        using on_finally_t = std::function<void()>;

        promise_t() : m_core(std::make_shared<promise_core>()) {}

        promise_t(std::nullptr_t) : m_core(nullptr) {}

        explicit operator bool() const { return bool(m_core); }

        self &then(std::nullptr_t) {
            m_core->then(nullptr);
            return *this;
        }

        self &then(std::function<void(const T &...)> f) {
            m_core->then([UVCXX_CAPTURE_MOVE(f)](void *p) {
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
                std::is_constructible<on_then_t, FUNC>::value, int>::type = 0>
        self &then(FUNC f) {
            return this->then(on_then_t(f));
        }

        template<typename FUNC, typename std::enable_if<
                !std::is_constructible<on_then_t, FUNC>::value && std::is_same<
                        decltype(std::declval<FUNC>()(std::declval<const T &>()...)),
                        void>::value, int>::type = 0>
        self &then(FUNC f) {
            return this->then(on_then_t([UVCXX_CAPTURE_MOVE(f)](const T &...args) {
                f(args...);
            }));
        }

        template<typename FUNC, typename std::enable_if<
                sizeof...(T) != 0 && std::is_same<
                        decltype(std::declval<FUNC>()()),
                        void>::value, int>::type = 0>
        self &then(FUNC f) {
            return this->then(on_then_t([UVCXX_CAPTURE_MOVE(f)](void *) {
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

        std::future<std::tuple<T...>> get_future() {
            auto sp = std::make_shared<std::promise<std::tuple<T...>>>();
            this->then([sp](const T &...v) {
                sp->set_value(std::make_tuple(v...));
            }).except([sp](std::exception_ptr p) {
                sp->set_exception(p);
            });
            return sp->get_future();
        }

    private:
        std::shared_ptr<promise_core> m_core;

        explicit promise_t(decltype(m_core) core) : m_core(std::move(core)) {}

    private:
        template<typename... K>
        friend
        class promise_emitter;

        template<typename... K>
        friend
        class promise_proxy;
    };

    template<typename... T>
    class promise_proxy {
    public:
        using self = promise_proxy;
        using type = std::tuple<T...>;

        virtual ~promise_proxy() = default;

        virtual void resolve(const T &...v) UVCXX_NOEXCEPT = 0;

        virtual void reject(std::exception_ptr p) UVCXX_NOEXCEPT = 0;

        virtual void finalize() UVCXX_NOEXCEPT = 0;

        template<typename E, typename... ARGS,
                typename std::enable_if<
                        std::is_base_of<std::exception, E>::value &&
                        std::is_constructible<E, ARGS...>::value, int>::type = 0>
        void reject(ARGS &&...args) UVCXX_NOEXCEPT {
            try { throw E(std::forward<ARGS>(args)...); }
            catch (...) { this->reject(std::current_exception()); }
        }

        void apply(const std::tuple<T...> &v) UVCXX_NOEXCEPT {
            proxy_apply([this](const T &...v) {
                this->resolve(v...);
            }, v);
        }

        void operator()(const T &...v) {
            this->resolve(v...);
        }
    };

    template<typename... T>
    class promise_emitter : public promise_proxy<T...> {
    public:
        using self = promise_emitter;
        using supper = promise_proxy<T...>;

        promise_emitter() : m_core(std::make_shared<promise_core>()) {}

        promise_emitter(std::nullptr_t) : m_core(nullptr) {}

        explicit promise_emitter(const promise_t<T...> &p)
                : m_core(p.m_core) {}

        explicit operator bool() const { return bool(m_core); }

        void resolve(const T &...v) UVCXX_NOEXCEPT final {
            std::tuple<T...> pack = std::make_tuple(v...);
            m_core->resolve(&pack);
        }

        using supper::reject;

        void reject() UVCXX_NOEXCEPT {
            this->reject(std::current_exception());
        }

        void reject(std::exception_ptr p) UVCXX_NOEXCEPT final {
            m_core->reject(p);
        }

        void finalize() UVCXX_NOEXCEPT final {
            m_core->finalize();
        }

        UVCXX_NODISCARD
        promise_t<T...> promise() const { return promise_t<T...>{m_core}; }

    private:
        std::shared_ptr<promise_core> m_core;
    };

    template<typename V, typename... T>
    class promise_cast;

    template<typename... V, typename... T>
    class promise_cast<promise_t<V...>, T...> : public promise_proxy<T...> {
    public:
        using self = promise_cast;
        using supper = promise_proxy<T...>;

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

        promise_cast(std::nullptr_t) : m_emitter(nullptr) {}

        promise_cast(const promise_t<V...> &p, wrapper_t wrapper)
                : m_emitter(p), m_wrapper(std::move(wrapper)) {}

        template<typename FUNC, typename std::enable_if<
                std::is_constructible<wrapper_t, FUNC>::value, int>::type = 0>
        promise_cast(const promise_t<V...> &p, FUNC wrapper)
                : self(p, wrapper_t(wrapper)) {}

        template<typename FUNC, typename std::enable_if<
                !std::is_constructible<wrapper_t, FUNC>::value &&
                sizeof...(V) == 1 && std::is_convertible<
                        decltype(std::declval<FUNC>()(std::declval<const T &>()...)),
                        typename first_element<value_t>::type>::value, int>::type = 0>
        promise_cast(const promise_t<V...> &p, FUNC wrapper)
                : self(p, wrapper_t([UVCXX_CAPTURE_MOVE(wrapper)](const T &...value) -> value_t {
            return std::make_tuple(wrapper(value...));
        })) {}

        template<typename FUNC, typename std::enable_if<
                !std::is_constructible<wrapper_t, FUNC>::value &&
                sizeof...(V) == 0 && std::is_same<
                        decltype(std::declval<FUNC>()(std::declval<const T &>()...)),
                        void>::value, int>::type = 0>
        promise_cast(const promise_t<V...> &p, FUNC wrapper)
                : self(p, wrapper_t([UVCXX_CAPTURE_MOVE(wrapper)](const T &...value) -> value_t {
            wrapper(value...);
            return std::make_tuple();
        })) {}

        template<typename FUNC, typename std::enable_if<
                std::is_constructible<self, const promise_t<V...> &, FUNC>::value, int>::type = 0>
        explicit promise_cast(FUNC wrapper)
                : self(promise_t<V...>(), std::move(wrapper)) {}

        explicit operator bool() const { return bool(m_emitter); }

        void resolve(const T &...v) UVCXX_NOEXCEPT final {
            try {
                auto tmp = m_wrapper(v...);
                m_emitter.apply(tmp);
            } catch (...) {
                self::reject(std::current_exception());
                return;
            }
        }

        using supper::reject;

        void reject() UVCXX_NOEXCEPT {
            this->reject(std::current_exception());
        }

        void reject(std::exception_ptr p) UVCXX_NOEXCEPT final {
            m_emitter.reject(p);
        }

        void finalize() UVCXX_NOEXCEPT final {
            m_emitter.finalize();
        }

        UVCXX_NODISCARD
        promise_t<V...> promise() const { return m_emitter.promise(); }

    private:
        promise_emitter<V...> m_emitter;
        wrapper_t m_wrapper;
    };

    template<typename...T>
    using promise = promise_t<T...>;
}

#endif //LIBUVCXX_PROMISE_H
