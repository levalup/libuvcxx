//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_BASE_H
#define LIBUVCXX_BASE_H

#include <memory>
#include <type_traits>

#include "../utils/standard.h"

namespace uvcxx {
    /**
     * The base class of all `libuvcxx` wrapper types.
     * It can be used for type detection in certain scenarios.
     */
    class base_t {
    };

    template<typename T>
    class shared_raw_base_t : public base_t {
    public:
        using self = shared_raw_base_t;
        using raw_t = T;

        shared_raw_base_t(const shared_raw_base_t &) = default;

        shared_raw_base_t &operator=(const shared_raw_base_t &) = default;

        shared_raw_base_t(shared_raw_base_t &&that) UVCXX_NOEXCEPT = default;

        shared_raw_base_t &operator=(shared_raw_base_t &&that) UVCXX_NOEXCEPT = default;

        shared_raw_base_t(std::nullptr_t) {}

        operator bool() const { return m_raw; }

        operator raw_t *() { return m_raw.get(); }

        operator raw_t *() const { return m_raw.get(); }

    protected:
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
                : m_raw(std::shared_ptr<raw_t>(borrow.raw, [](raw_t *) {})) {}

        template<typename K>
        static inline std::shared_ptr<raw_t> cast_raw(const std::shared_ptr<K> &r) {
            auto p = reinterpret_cast<typename std::shared_ptr<raw_t>::element_type *>(r.get());
            return std::shared_ptr<raw_t>{r, p};
        }

    private:
        std::shared_ptr<raw_t> m_raw;
    };

    template<typename T>
    class pointer_raw_base_t : public base_t {
    public:
        using self = pointer_raw_base_t;
        using raw_t = T;

        pointer_raw_base_t(const pointer_raw_base_t &) = delete;

        pointer_raw_base_t &operator=(const pointer_raw_base_t &) = delete;

        pointer_raw_base_t(pointer_raw_base_t &&that) UVCXX_NOEXCEPT {
            this->operator=(std::move(that));
        }

        pointer_raw_base_t &operator=(pointer_raw_base_t &&that) UVCXX_NOEXCEPT {
            std::swap(m_raw, that.m_raw);
            return *this;
        };

        pointer_raw_base_t(std::nullptr_t) {}

        pointer_raw_base_t()
                : m_raw(new raw_t()) {}

        virtual ~pointer_raw_base_t() {
            delete m_raw;
        }

        operator bool() const { return m_raw; }

        operator raw_t *() { return m_raw; }

        operator raw_t *() const { return m_raw; }

    protected:
        raw_t *raw() { return m_raw; }

        raw_t *raw() const { return m_raw; }

        template<typename K>
        K *raw() { return (K *) m_raw; }

        template<typename K>
        K *raw() const { return (K *) m_raw; }

        raw_t *reset_raw(raw_t *raw) {
            auto pre = m_raw;
            m_raw = raw;
            return pre;
        }

        raw_t *reset_raw(raw_t *raw, void(*del)(raw_t *)) {
            auto pre = m_raw;
            m_raw = raw;
            if (pre && del) {
                del(pre);
                return nullptr;
            } else {
                return pre;
            }
        }

    private:
        raw_t *m_raw = nullptr;
    };

    template<typename T>
    class inherit_raw_base_t : public base_t, public T {
    public:
        using self = inherit_raw_base_t;
        using supper = T;
        using raw_t = T;

        inherit_raw_base_t(const inherit_raw_base_t &) = default;

        inherit_raw_base_t &operator=(const inherit_raw_base_t &) = default;

        inherit_raw_base_t(inherit_raw_base_t &&that) UVCXX_NOEXCEPT {
            this->operator=(std::move(that));
        }

        inherit_raw_base_t &operator=(inherit_raw_base_t &&that) UVCXX_NOEXCEPT {
            std::swap(*((T *) this), *((T *) &that));
            return *this;
        };

        inherit_raw_base_t() : supper() {}

        explicit inherit_raw_base_t(raw_t raw) : supper(std::move(raw)) {}

        operator raw_t *() { return this; }

        operator raw_t *() const { return const_cast<self *>(this); }

    protected:
        raw_t *raw() { return (raw_t *) (*this); }

        raw_t *raw() const { return (raw_t *) (*this); }

        template<typename K>
        K *raw() { return (K *) (raw()); }

        template<typename K>
        K *raw() const { return (K *) (raw()); }
    };

    template<typename T>
    class extend_raw_base_t : public base_t {
    public:
        using self = extend_raw_base_t;
        using raw_t = T;

        extend_raw_base_t(const extend_raw_base_t &) = default;

        extend_raw_base_t &operator=(const extend_raw_base_t &) = default;

        extend_raw_base_t(extend_raw_base_t &&that) UVCXX_NOEXCEPT {
            this->operator=(std::move(that));
        }

        extend_raw_base_t &operator=(extend_raw_base_t &&that) UVCXX_NOEXCEPT {
            std::swap(m_raw, that.m_raw);
            return *this;
        };

        extend_raw_base_t() = default;

        explicit extend_raw_base_t(raw_t raw) : m_raw(std::move(raw)) {}

        operator raw_t *() { return &m_raw; }

        operator raw_t *() const { return const_cast<raw_t *>(&m_raw); }

    protected:
        raw_t *raw() { return (raw_t *) (*this); }

        raw_t *raw() const { return (raw_t *) (*this); }

        template<typename K>
        K *raw() { return (K *) (raw()); }

        template<typename K>
        K *raw() const { return (K *) (raw()); }

    private:
        raw_t m_raw{};
    };
}

#endif //LIBUVCXX_BASE_H
