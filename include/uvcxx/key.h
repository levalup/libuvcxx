//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_KEY_H
#define LIBUVCXX_KEY_H

#include <uv.h>

#include "cxx/except.h"
#include "inner/base.h"

namespace uv {
    class key_t : public uvcxx::extend_raw_base_t<uv_key_t> {
    public:
        using self = key_t;
        using supper = uvcxx::extend_raw_base_t<uv_key_t>;

        key_t(const key_t &) = delete;

        key_t &operator=(const key_t &) = delete;

        key_t(key_t &&that) noexcept = default;

        key_t &operator=(key_t &&that) noexcept = default;

        key_t() {
            auto err = uv_key_create(*this);
            if (err < 0) throw uvcxx::errcode(err, "can not create key");
        }

        ~key_t() {
            uv_key_delete(*this);
        }

        void set(void *value) {
            uv_key_set(*this, value);
        }

        [[nodiscard]]
        void *get() const {
            return uv_key_get(*this);
        }

        template<typename T>
        [[nodiscard]]
        T *get() const {
            return (T *) uv_key_get(*this);
        }

        template<typename T>
        [[nodiscard]]
        T &ref() const {
            return *((T *) uv_key_get(*this));
        }
    };
}

#endif //LIBUVCXX_KEY_H
