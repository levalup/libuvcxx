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

namespace uvcxx {
    class promise_core {
    public:
        using self = promise_core;

        using on_then_t = std::function<void(void *)>;
        // Return whether this exception has been handled
        using on_except_t = std::function<bool(std::exception_ptr)>;
        using on_finally_t = std::function<void()>;

        ~promise_core() {
            emit_finally();
        }

        void resolve(void *p) noexcept {
            try {
                if (m_on_then) m_on_then(p);
            } catch (...) {
                reject(std::current_exception());
            }
        }

        void reject(std::exception_ptr p) noexcept {
            try {
                if (m_on_except) m_on_except(p);
                else default_on_except(p);
            } catch (...) {
                default_on_except(p);
            }
        }

        void emit_finally() noexcept {
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
            m_on_then =
                    m_on_then
                    ? [f = std::move(f), pre = std::move(m_on_then)](void *p) {
                        pre(p);
                        f(p);
                    }
                    : std::move(f);
            return *this;
        }

        self &except(std::nullptr_t) {
            m_on_except = nullptr;
            return *this;
        }

        self &except(on_except_t f) {
            if (!f) return *this;
            m_on_except =
                    m_on_except
                    ? [f = std::move(f), pre = std::move(m_on_except)](std::exception_ptr p) {
                        if (pre(p)) return true;
                        return f(p);
                    }
                    : std::move(f);
            return *this;
        }

        self &finally(std::nullptr_t) {
            m_on_finally = nullptr;
            return *this;
        }

        self &finally(on_finally_t f) {
            if (!f) return *this;
            m_on_finally =
                    m_on_finally
                    ? [f = std::move(f), pre = std::move(m_on_finally)]() {
                        pre();
                        f();
                    }
                    : std::move(f);
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
            m_core->then([f = std::move(f)](void *p) {
                auto &pack = *(const type *) (p);
                std::apply(f, pack);
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

        template<typename E, typename std::enable_if_t<
                std::is_base_of_v<std::exception, E>, int> = 0>
        self &except(std::function<void(const E &)> f) {
            m_core->except([f = std::move(f)](std::exception_ptr p) -> bool {
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

        template<typename E, typename std::enable_if_t<
                std::is_base_of_v<std::exception, E>, int> = 0>
        self &except(std::function<void()> f) {
            return this->except<E>([f = std::move(f)](const E &) { f(); });
        }

        self &finally(std::nullptr_t) {
            m_core->finally(nullptr);
            return *this;
        }

        self &finally(std::function<void()> f) {
            m_core->finally(std::move(f));
            return *this;
        }

        template<typename FUNC>
        typename std::enable_if_t<std::is_constructible_v<on_then_t, FUNC>, self> &
        then(FUNC f) {
            return this->then(on_then_t(f));
        }

        template<typename FUNC>
        typename std::enable_if_t<
                !std::is_constructible_v<on_then_t, FUNC> && std::is_same_v<
                        decltype(std::declval<FUNC>()(std::declval<const T &>()...)),
                        void>, self> &
        then(FUNC f) {
            return this->then(on_then_t([f = std::move(f)](const T &...args) {
                f(args...);
            }));
        }

        template<typename FUNC>
        typename std::enable_if_t<
                sizeof...(T) != 0 && std::is_same_v<
                        decltype(std::declval<FUNC>()()),
                        void>, self> &
        then(FUNC f) {
            return this->then(on_then_t([f = std::move(f)](void *) {
                f();
            }));
        }

        template<typename FUNC>
        typename std::enable_if_t<std::is_same_v<
                decltype(std::declval<FUNC>()(std::declval<std::exception_ptr>())),
                void>, self> &
        except(FUNC f) {
            return this->except(on_except_t([f = std::move(f)](std::exception_ptr p) -> bool {
                f(p);
                return false;
            }));
        }

        template<typename E, typename FUNC>
        typename std::enable_if_t<std::is_same_v<
                decltype(std::declval<FUNC>()(std::declval<const E &>())),
                void>, self> &
        except(FUNC f) {
            return this->except<E>(std::function<void(const E &)>(std::move(f)));
        }

        template<typename E, typename FUNC>
        typename std::enable_if_t<std::is_same_v<
                decltype(std::declval<FUNC>()()),
                void>, self> &
        except(FUNC f) {
            return this->except<E>(std::function<void()>(std::move(f)));
        }

        template<typename FUNC>
        typename std::enable_if_t<std::is_same_v<
                decltype(std::declval<FUNC>()()),
                void>, self> &
        finally(FUNC f) {
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

        virtual void resolve(const T &...v) noexcept = 0;

        virtual void reject(std::exception_ptr p) noexcept = 0;

        virtual void finalize() noexcept = 0;

        template<typename E, typename... ARGS,
                typename = typename std::enable_if_t<
                        std::is_base_of_v<std::exception, E> && std::is_constructible_v<E, ARGS...>
                >>
        void reject(ARGS &&...args) noexcept {
            try { throw E(std::forward<ARGS>(args)...); }
            catch (...) { this->reject(std::current_exception()); }
        }

        void apply(const std::tuple<T...> &v) noexcept {
            std::apply([this](const T &...v) {
                this->resolve(v...);
            }, v);
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

        void resolve(const T &...v) noexcept final {
            std::tuple<T...> pack = std::make_tuple(v...);
            m_core->resolve(&pack);
        }

        using supper::reject;

        void reject() noexcept {
            this->reject(std::current_exception());
        }

        void reject(std::exception_ptr p) noexcept final {
            m_core->reject(p);
        }

        void finalize() noexcept final {
            m_core->emit_finally();
        }

        [[nodiscard]]
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

        template<typename FUNC, std::enable_if_t<
                std::is_constructible_v<wrapper_t, FUNC>, int> = 0>
        promise_cast(const promise_t<V...> &p, FUNC wrapper)
                : self(p, wrapper_t(wrapper)) {}

        template<typename FUNC, std::enable_if_t<
                !std::is_constructible_v<wrapper_t, FUNC> &&
                sizeof...(V) == 1 && std::is_convertible_v<
                        decltype(std::declval<FUNC>()(std::declval<const T &>()...)),
                        typename first_element<value_t>::type>, int> = 0>
        promise_cast(const promise_t<V...> &p, FUNC wrapper)
                : self(p, wrapper_t([wrapper = std::move(wrapper)](const T &...value) -> value_t {
            return std::make_tuple(wrapper(value...));
        })) {}

        template<typename FUNC, std::enable_if_t<
                !std::is_constructible_v<wrapper_t, FUNC> &&
                sizeof...(V) == 0 && std::is_same_v<
                        decltype(std::declval<FUNC>()(std::declval<const T &>()...)),
                        void>, int> = 0>
        promise_cast(const promise_t<V...> &p, FUNC wrapper)
                : self(p, wrapper_t([wrapper = std::move(wrapper)](const T &...value) -> value_t {
            wrapper(value...);
            return std::make_tuple();
        })) {}

        template<typename FUNC, std::enable_if_t<
                std::is_constructible_v<self, const promise_t<V...> &, FUNC>, int> = 0>

        explicit promise_cast(FUNC wrapper)
                : self(promise_t<V...>(), wrapper_t(wrapper)) {}

        explicit operator bool() const { return bool(m_emitter); }

        void resolve(const T &...v) noexcept final {
            try {
                auto tmp = m_wrapper(v...);
                m_emitter.apply(tmp);
            } catch (...) {
                self::reject(std::current_exception());
                return;
            }
        }

        using supper::reject;

        void reject() noexcept {
            this->reject(std::current_exception());
        }

        void reject(std::exception_ptr p) noexcept final {
            m_emitter.reject(p);
        }

        void finalize() noexcept final {
            m_emitter.finalize();
        }

        [[nodiscard]]
        promise_t<V...> promise() const { return m_emitter.promise(); }

    private:
        promise_emitter<V...> m_emitter;
        wrapper_t m_wrapper;
    };

    template<typename...T>
    using promise = promise_t<T...>;
}

#endif //LIBUVCXX_PROMISE_H
