//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_PROMISE_H
#define LIBUVCXX_PROMISE_H

#include <functional>
#include <memory>
#include <iostream>
#include <sstream>
#include <utility>

namespace uvcxx {
    static void default_on_except(const std::exception &e) {
        std::ostringstream oss;
        oss << "[ERROR] " << e.what() << std::endl;
        std::cerr << oss.str();
    }

    static void default_on_except(std::exception_ptr ptr) {
        try {
            std::rethrow_exception(ptr);
        } catch (const std::exception &e) {
            default_on_except(e);
        }
    }

    class promise_core {
    public:
        using on_then_t = std::function<void(void *)>;
        using on_except_t = std::function<void(std::exception_ptr)>;
        using on_finally_t = std::function<void()>;

        void emit_then(void *p) noexcept {
            try {
                if (m_on_then) m_on_then(p);
            } catch (...) {
                emit_except(std::current_exception());
            }
        }

        void emit_except(std::exception_ptr ptr) noexcept {
            try {
                if (m_on_except) m_on_except(ptr);
                else default_on_except(ptr);
            } catch (const std::exception &e) {
                default_on_except(e);
            }
        }

        void emit_finally() noexcept {
            try {
                if (m_on_finally) m_on_finally();
            } catch (const std::exception &e) {
                default_on_except(e);
            }
        }

        promise_core &then(on_then_t f) {
            m_on_then = std::move(f);
            return *this;
        }

        promise_core &except(on_except_t f) {
            m_on_except = std::move(f);
            return *this;
        }

        promise_core &finally(on_finally_t f) {
            m_on_finally = std::move(f);
            return *this;
        }

    private:
        on_then_t m_on_then;
        on_except_t m_on_except;
        on_finally_t m_on_finally;
    };

    template<typename T>
    class promise {
    public:
        using self = promise;
        using type = T;

        template<typename P>
        struct param {
            using type = const P &;
        };

        template<>
        struct param<void> {
            using type = void;
        };

        template<typename P>
        struct on_then {
            using declare = void(const P &);
        };

        template<>
        struct on_then<void> {
            using declare = void();
        };

        using on_then_t = std::function<typename on_then<T>::declare>;
        using on_except_p_t = std::function<void(std::exception_ptr)>;
        using on_except_v_t = std::function<void(const std::exception &)>;
        using on_finally_t = std::function<void()>;

        promise() : m_core(std::make_shared<promise_core>()) {}

        self &then(on_then_t on_then) {
            m_core->then([on_then = std::move(on_then)](void *p) {
                f(*(const T *) (p));
            });
            return *this;
        }

        self &except(on_except_p_t on_except) {
            m_core->except(std::move(on_except));
            return *this;
        }

        self &except(on_except_v_t on_except) {
            m_core->except([on_except = std::move(on_except)](std::exception_ptr ptr) {
                try {
                    std::rethrow_exception(ptr);
                } catch (const std::exception &e) {
                    on_except(e);
                }
            });
            return *this;
        }

        self &finally(on_finally_t on_finally) {
            m_core->finally(std::move(on_finally));
            return *this;
        }

        template<typename FUNC>
        typename std::enable_if_t<!std::is_same_v<T, void> && std::is_same_v<
                decltype(std::declval<FUNC>()(std::declval<typename param<T>::type>())),
                void>, self> &
        then(FUNC f) {
            m_core->then([f = std::move(f)](void *p) {
                f(*(const T *) (p));
            });
            return *this;
        }

        template<typename FUNC>
        typename std::enable_if_t<std::is_same_v<
                decltype(std::declval<FUNC>()()),
                void>, self> &
        then(FUNC f) {
            m_core->then([f = std::move(f)](void *p) {
                f();
            });
            return *this;
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

    private:
        std::shared_ptr<promise_core> m_core;

    protected:
        promise_core &core() { return *m_core; }

    private:
        template<typename K>
        friend class promise_emitter;
        template<typename K>
        friend class promise_proxy;
    };

    template <typename T>
    class promise_proxy {
    public:
        using self = promise_proxy;
        using type = T;

        virtual ~promise_proxy() = default;

        virtual void resolve(const T &value) noexcept = 0;

        virtual void reject(std::exception_ptr ptr) noexcept = 0;

        virtual void finally() noexcept = 0;
    };

    template <>
    class promise_proxy<void> {
    public:
        using self = promise_proxy;
        using type = void;

        virtual ~promise_proxy() = default;

        virtual void resolve() noexcept = 0;

        virtual void reject(std::exception_ptr ptr) noexcept = 0;

        virtual void finally() noexcept = 0;
    };

    template <typename T>
    class promise_emitter : public promise_proxy<T> {
    public:
        using self = promise_emitter;

        explicit promise_emitter(const promise<T> &p)
            : m_core(p.m_core) {}

        void resolve() noexcept {
            if constexpr (std::is_default_constructible_v<T>) {
                T tmp;
                m_core->emit_then(&tmp);
            } else {
                static_assert(std::is_default_constructible_v<T>, "only support emit default constructible object");
            }
        }

        void resolve(const T &value) noexcept final {
            m_core->emit_then((T *)&value);
        }

        void reject() noexcept {
            this->reject(std::current_exception());
        }

        void reject(std::exception_ptr ptr) noexcept final {
            m_core->emit_except(ptr);
        }

        void finally() noexcept final {
            m_core->emit_finally();
        }

    private:
        std::shared_ptr<promise_core> m_core;

    protected:
        promise_core &core() { return *m_core; }
    };


    template <>
    class promise_emitter<void> : public promise_proxy<void> {
    public:
        using self = promise_emitter;

        explicit promise_emitter(const promise<void> &p)
                : m_core(p.m_core) {}

        void resolve() noexcept final {
            m_core->emit_then(this);
        }

        template<typename K>
        void resolve(const K &) noexcept {
            m_core->emit_then(this);
        }

        void reject() noexcept {
            this->reject(std::current_exception());
        }

        void reject(std::exception_ptr ptr) noexcept final {
            m_core->emit_except(ptr);
        }

        void finally() noexcept final {
            m_core->emit_finally();
        }

    private:
        std::shared_ptr<promise_core> m_core;

    protected:
        promise_core &core() { return *m_core; }
    };

    template <typename TO, typename FROM>
    class promise_cast : public promise_proxy<FROM> {
    public:
        using self = promise_cast;
        using supper = promise_proxy<FROM>;

        using wrapper_t = std::function<TO(FROM)>;

        promise_cast(promise<TO> &p, wrapper_t wrapper)
            : m_emitter(p)
                , m_wrapper(std::move(wrapper)) {}

        template<typename FUNC, typename=typename std::enable_if_t<std::is_convertible_v<
                decltype(std::declval<FUNC>()(std::declval<const FROM &>())),
                TO>>>
        promise_cast(promise<TO> &p, FUNC wrapper)
                : self(p, wrapper_t(wrapper)) {}

        void resolve(const FROM &value) noexcept final {
            try {
                m_emitter.resolve(m_wrapper(value));
            } catch (...) {
                self::reject(std::current_exception());
                return;
            }
        }

        void reject() noexcept {
            this->reject(std::current_exception());
        }

        void reject(std::exception_ptr ptr) noexcept final {
            m_emitter.reject(ptr);
        }

        void finally() noexcept final {
            m_emitter.finally();
        }

    private:
        promise_emitter<TO> m_emitter;
        wrapper_t m_wrapper;
    };

    template <typename FROM>
    class promise_cast<void, FROM> : public promise_proxy<FROM> {
    public:
        using self = promise_cast;
        using supper = promise_proxy<FROM>;

        using wrapper_t = std::function<void(FROM)>;

        promise_cast(promise<void> &p, wrapper_t wrapper)
                : m_emitter(p)
                , m_wrapper(std::move(wrapper)) {}

        template<typename FUNC, typename=typename std::enable_if_t<std::is_same_v<
                decltype(std::declval<FUNC>()(std::declval<const FROM &>())),
                void>>>
        promise_cast(promise<void> &p, FUNC wrapper)
                : self(p, wrapper_t(wrapper)) {}

        explicit promise_cast(promise<void> &p)
                : m_emitter(p) {}

        void resolve(const FROM &value) noexcept final {
            try {
                if (m_wrapper) m_wrapper(value);
                m_emitter.resolve();
            } catch (...) {
                self::reject(std::current_exception());
                return;
            }
        }

        void reject() noexcept {
            this->reject(std::current_exception());
        }

        void reject(std::exception_ptr ptr) noexcept final {
            m_emitter.reject(ptr);
        }

        void finally() noexcept final {
            m_emitter.finally();
        }

    private:
        promise_emitter<void> m_emitter;
        wrapper_t m_wrapper;
    };

    template <typename TO>
    class promise_cast<TO, void> final : public promise_proxy<void> {
    public:
        using self = promise_cast;
        using supper = promise_proxy<void>;

        using wrapper_t = std::function<TO(void)>;

        promise_cast(promise<TO> &p, wrapper_t wrapper)
                : m_emitter(p)
                , m_wrapper(std::move(wrapper)) {}

        template<typename FUNC, typename=typename std::enable_if_t<std::is_convertible_v<
                decltype(std::declval<FUNC>()()),
                TO>>>
        promise_cast(promise<TO> &p, FUNC wrapper)
                : self(p, wrapper_t(wrapper)) {}

        explicit promise_cast(promise<TO> &p)
                : m_emitter(p) {}

        void resolve() noexcept final {
            try {
                auto tmp = m_wrapper ? m_wrapper() : TO{};
                m_emitter.resolve(tmp);
            } catch (...) {
                self::reject(std::current_exception());
                return;
            }
        }

        void reject() noexcept {
            self::reject(std::current_exception());
        }

        void reject(std::exception_ptr ptr) noexcept final {
            m_emitter.reject(ptr);
        }

        void finally() noexcept final {
            m_emitter.finally();
        }

    private:
        promise_emitter<TO> m_emitter;
        wrapper_t m_wrapper;
    };

    template <>
    class promise_cast<void, void> final : public promise_proxy<void> {
    public:
        using self = promise_cast;
        using supper = promise_proxy<void>;

        using wrapper_t = std::function<void(void)>;

        promise_cast(promise<void> &p, wrapper_t wrapper)
                : m_emitter(p)
                , m_wrapper(std::move(wrapper)) {}

        template<typename FUNC, typename=typename std::enable_if_t<std::is_same_v<
                decltype(std::declval<FUNC>()()),
                void>>>
        promise_cast(promise<void> &p, FUNC wrapper)
                : self(p, wrapper_t(wrapper)) {}

        explicit promise_cast(promise<void> &p)
                : m_emitter(p) {}

        void resolve() noexcept final {
            try {
                if (m_wrapper) m_wrapper();
                m_emitter.resolve();
            } catch (...) {
                self::reject(std::current_exception());
                return;
            }
        }

        void reject() noexcept {
            self::reject(std::current_exception());
        }

        void reject(std::exception_ptr ptr) noexcept final {
            m_emitter.reject(ptr);
        }

        void finally() noexcept final {
            m_emitter.finally();
        }

    private:
        promise_emitter<void> m_emitter;
        wrapper_t m_wrapper;
    };
}

#endif //LIBUVCXX_PROMISE_H
