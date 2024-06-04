//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CALLBACK_H
#define LIBUVCXX_CALLBACK_H

#include <functional>
#include <memory>
#include <iostream>
#include <sstream>
#include <utility>
#include <future>

#include "queue.h"

namespace uvcxx {
    class callback_core {
    public:
        using self = callback_core;

        using on_call_t = std::function<void(void *)>;
        using on_except_t = std::function<void(std::exception_ptr)>;
        using on_finally_t = std::function<void()>;

        ~callback_core() {
            emit_finally();
        }

        void emit(void *p) noexcept {
            try {
                if (m_on_call) m_on_call(p);
            } catch (...) {
                raise(std::current_exception());
            }
        }

        void raise(std::exception_ptr p) noexcept {
            try {
                if (m_on_except) m_on_except(p);
                else default_on_except(p);
            } catch (const std::exception &e) {
                default_on_except(e);
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

        self &call(on_call_t f) {
            m_on_call = std::move(f);
            return *this;
        }

        self &except(on_except_t f) {
            m_on_except = std::move(f);
            return *this;
        }

        self &finally(on_finally_t f) {
            m_on_finally = std::move(f);
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
    class callback {
    public:
        using self = callback;
        using type = std::tuple<T...>;

        using on_call_t = std::function<void(const T &...)>;
        using on_except_p_t = std::function<void(std::exception_ptr)>;
        using on_except_v_t = std::function<void(const std::exception &)>;
        using on_finally_t = std::function<void()>;

        callback() : m_core(std::make_shared<callback_core>()) {}

        callback(std::nullptr_t) : m_core(nullptr) {}

        explicit operator bool() const { return bool(m_core); }

        self &call(std::function<void(const T &...)> f) {
            m_core->call([f = std::move(f)](void *p) {
                auto &pack = *(const type *) (p);
                std::apply(f, pack);
            });
            return *this;
        }

        self &except(std::nullptr_t) {
            m_core->except(nullptr);
            return *this;
        }

        self &except(std::function<void(std::exception_ptr)> f) {
            m_core->except(std::move(f));
            return *this;
        }

        self &except(std::function<void(const std::exception &)> f) {
            m_core->except([f = std::move(f)](std::exception_ptr p) {
                try {
                    std::rethrow_exception(p);
                } catch (const std::exception &e) {
                    f(e);
                }
            });
            return *this;
        }

        self &finally(std::function<void()> f) {
            m_core->finally(std::move(f));
            return *this;
        }

        template<typename FUNC>
        typename std::enable_if_t<std::is_constructible_v<on_call_t, FUNC>, self> &
        call(FUNC f) {
            return this->call(on_call_t(f));
        }

        template<typename FUNC>
        typename std::enable_if_t<
                !std::is_constructible_v<on_call_t, FUNC> && std::is_same_v<
                        decltype(std::declval<FUNC>()(std::declval<const T &>()...)),
                        void>, self> &
        call(FUNC f) {
            return this->call(on_call_t([f = std::move(f)](const T &...args) {
                f(args...);
            }));
        }

        template<typename FUNC>
        typename std::enable_if_t<
                sizeof...(T) != 0 && std::is_same_v<
                        decltype(std::declval<FUNC>()()),
                        void>, self> &
        call(FUNC f) {
            return this->call(on_call_t([f = std::move(f)](void *) {
                f();
            }));
        }

        template<typename FUNC>
        typename std::enable_if_t<std::is_same_v<
                decltype(std::declval<FUNC>()(std::declval<std::exception_ptr>())),
                void>, self> &
        except(FUNC f) {
            return this->except(on_except_p_t(f));
        }

        template<typename FUNC>
        typename std::enable_if_t<std::is_same_v<
                decltype(std::declval<FUNC>()(std::declval<const std::exception &>())),
                void>, self> &
        except(FUNC f) {
            return this->except(on_except_v_t(f));
        }

        template<typename FUNC>
        typename std::enable_if_t<std::is_same_v<
                decltype(std::declval<FUNC>()()),
                void>, self> &
        finally(FUNC f) {
            return this->finally(on_finally_t(f));
        }

        queue<T...> get_queue() {
            queue<T...> q;
            this->call([q = q](const T &...v) mutable {
                q.push(v...);
            }).finally([q = q]() mutable {
                q.close();
            }).except(nullptr);
            return q;
        }

    private:
        std::shared_ptr<callback_core> m_core;

        explicit callback(decltype(m_core) core) : m_core(std::move(core)) {}

    private:
        template<typename... K>
        friend
        class callback_emitter;

        template<typename... K>
        friend
        class callback_proxy;
    };

    template<>
    class callback<void> : public callback<> {
    public:
        using self = callback;
        using supper = callback<>;

        using supper::supper;
    };

    template<typename...T>
    using callback_t = callback<T...>;

    template<typename... T>
    class callback_proxy {
    public:
        using self = callback_proxy;
        using type = std::tuple<T...>;

        virtual ~callback_proxy() = default;

        virtual void emit(const T &...v) noexcept = 0;

        virtual void raise(std::exception_ptr p) noexcept = 0;

        virtual void finally() noexcept = 0;

        void apply(const std::tuple<T...> &v) noexcept {
            std::apply([this](const T &...v) {
                this->emit(v...);
            }, v);
        }

        void operator()(const T &...v) {
            this->emit(v...);
        }
    };

    template<>
    class callback_proxy<void> : public callback_proxy<> {
    };

    template<typename... T>
    class callback_emitter : public callback_proxy<T...> {
    public:
        using self = callback_emitter;
        using supper = callback_proxy<T...>;

        callback_emitter() : m_core(std::make_shared<callback_core>()) {}

        callback_emitter(std::nullptr_t) : m_core(nullptr) {}

        explicit callback_emitter(const callback<T...> &p)
                : m_core(p.m_core) {}

        explicit operator bool() const { return bool(m_core); }

        void emit(const T &...v) noexcept final {
            std::tuple<T...> pack = std::make_tuple(v...);
            m_core->emit(&pack);
        }

        void raise() noexcept {
            this->raise(std::current_exception());
        }

        void raise(std::exception_ptr p) noexcept final {
            m_core->raise(p);
        }

        void finally() noexcept final {
            m_core->emit_finally();
        }

        [[nodiscard]]
        callback<T...> callback() const { return callback_t<T...>{m_core}; }

    private:
        std::shared_ptr<callback_core> m_core;
    };

    template<>
    class callback_emitter<void> : public callback_emitter<> {
        using self = callback_emitter;
        using supper = callback_emitter<>;

        using supper::supper;
    };

    template<typename V, typename... T>
    class callback_cast;

    template<typename... V, typename... T>
    class callback_cast<callback<V...>, T...> : public callback_proxy<T...> {
    public:
        using self = callback_cast;
        using supper = callback_proxy<T...>;

        using value_t = std::tuple<V...>;
        using wrapper_t = std::function<value_t(const T &...)>;

        template <typename Tuple>
        struct first_element;

        template <typename First, typename ...Else>
        struct first_element<std::tuple<First, Else...>> {
            using type = First;
        };

        template <>
        struct first_element<std::tuple<>> {
            using type = void;
        };

        template <typename Tuple>
        using first_element_t = typename first_element<Tuple>::type;

        callback_cast(std::nullptr_t) : m_emitter(nullptr) {}

        callback_cast(const callback<V...> &p, wrapper_t wrapper)
                : m_emitter(p), m_wrapper(std::move(wrapper)) {}

        template<typename FUNC, std::enable_if_t<
                std::is_constructible_v<wrapper_t, FUNC>, int> = 0>
        callback_cast(const callback<V...> &p, FUNC wrapper)
                : self(p, wrapper_t(wrapper)) {}

        template<typename FUNC, std::enable_if_t<
                !std::is_constructible_v<wrapper_t, FUNC> &&
                sizeof...(V) == 1 && std::is_convertible_v<
                        decltype(std::declval<FUNC>()(std::declval<const T &>()...)),
                        first_element_t<value_t>>, int> = 0>
        callback_cast(const callback<V...> &p, FUNC wrapper)
                : self(p, wrapper_t([wrapper = std::move(wrapper)](const T &...value) -> value_t {
            return std::make_tuple(wrapper(value...));
        })) {}

        template<typename FUNC, std::enable_if_t<
                !std::is_constructible_v<wrapper_t, FUNC> &&
                sizeof...(V) == 0 && std::is_same_v<
                        decltype(std::declval<FUNC>()(std::declval<const T &>()...)),
                        void>, int> = 0>
        callback_cast(const callback<V...> &p, FUNC wrapper)
                : self(p, wrapper_t([wrapper = std::move(wrapper)](const T &...value) -> value_t {
            wrapper(value...);
            return std::make_tuple();
        })) {}

        template<typename FUNC, std::enable_if_t<
                std::is_constructible_v<self, const callback<V...> &, FUNC>, int> = 0>
        explicit callback_cast(FUNC wrapper)
                : self(callback_t<V...>(), wrapper_t(wrapper)) {}

        explicit operator bool() const { return bool(m_emitter); }

        void emit(const T &...v) noexcept final {
            try {
                auto tmp = m_wrapper(v...);
                m_emitter.apply(tmp);
            } catch (...) {
                self::raise(std::current_exception());
                return;
            }
        }

        void raise() noexcept {
            this->raise(std::current_exception());
        }

        void raise(std::exception_ptr p) noexcept final {
            m_emitter.raise(p);
        }

        void finally() noexcept final {
            m_emitter.finally();
        }

        [[nodiscard]]
        callback<V...> callback() const { return m_emitter.callback(); }

    private:
        callback_emitter<V...> m_emitter;
        wrapper_t m_wrapper;
    };

    template<typename... T>
    class callback_cast<callback<void>, T...> : callback_cast<callback<>, T...> {
    public:
        using self = callback_cast;
        using supper = callback_cast<callback<>, T...>;

        using supper::supper;
    };
}

#endif //LIBUVCXX_CALLBACK_H
