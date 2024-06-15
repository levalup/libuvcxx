//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_BASE_H
#define LIBUVCXX_BASE_H

#include <memory>
#include <type_traits>

namespace uvcxx {
    template<typename T>
    class shared_raw_base_t {
    public:
        using self = shared_raw_base_t;
        using raw_t = T;

        shared_raw_base_t(std::nullptr_t) {}

        operator bool() const { return m_raw; }

        operator raw_t *() { return m_raw.get(); }

        operator raw_t *() const { return m_raw.get(); }

        raw_t *raw() { return m_raw.get(); }

        raw_t *raw() const { return m_raw.get(); }

        template<typename K>
        K *raw() { return (K *) (m_raw.get()); }

        template<typename K>
        K *raw() const { return (K *) (m_raw.get()); }

        const std::shared_ptr<raw_t> &shared_raw() const { return m_raw; }

    protected:
        class borrow_t {
        public:
            raw_t *raw;

            explicit borrow_t(raw_t *raw) : raw(raw) {}

            template<typename K>
            explicit borrow_t(K *raw) : raw((raw_t *) raw) {}
        };

        explicit shared_raw_base_t(std::shared_ptr<raw_t> raw)
                : m_raw(std::move(raw)) {}

        explicit shared_raw_base_t(borrow_t borrow)
                : m_raw(std::shared_ptr<raw_t>(borrow.raw, [](T *) {})) {}

        template<typename K>
        static std::shared_ptr<raw_t> cast_raw(const std::shared_ptr<K> &p) {
            return std::reinterpret_pointer_cast<raw_t>(p);
        }

    private:
        std::shared_ptr<raw_t> m_raw;
    };

    template<typename T>
    struct is_shared_raw {
    private:
        template<typename K>
        static std::true_type check(const shared_raw_base_t<K> &) { return {}; }

        static std::false_type check(...) { return {}; }

    public:
        static constexpr auto value = decltype(check(std::declval<T>()))::value;
    };

    template<typename T>
    class inherit_raw_base_t : public T {
    public:
        using self = inherit_raw_base_t;
        using raw_t = T;

        operator raw_t *() { return this; }

        operator raw_t *() const { return const_cast<self *>(this); }

        raw_t *raw() { return (raw_t *) (*this); }

        raw_t *raw() const { return (raw_t *) (*this); }

        template<typename K>
        K *raw() { return (K *) (raw()); }

        template<typename K>
        K *raw() const { return (K *) (raw()); }
    };

    template<typename T>
    class extend_raw_base_t : public T {
    public:
        using self = extend_raw_base_t;
        using raw_t = T;

        operator raw_t *() { return &m_raw; }

        operator raw_t *() const { return const_cast<raw_t *>(&m_raw); }

        raw_t *raw() { return (raw_t *) (*this); }

        raw_t *raw() const { return (raw_t *) (*this); }

        template<typename K>
        K *raw() { return (K *) (raw()); }

        template<typename K>
        K *raw() const { return (K *) (raw()); }

    private:
        T m_raw{};
    };
}

#endif //LIBUVCXX_BASE_H
