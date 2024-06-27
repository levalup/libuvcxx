//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_KEY_H
#define LIBUVCXX_KEY_H

#include <uv.h>

#include "cxx/except.h"
#include "cxx/wrapper.h"
#include "inner/base.h"

namespace uv {
    class key_t : public uvcxx::pointer_raw_base_t<uv_key_t> {
    public:
        using self = key_t;
        using supper = uvcxx::pointer_raw_base_t<uv_key_t>;

        using supper::supper;

        key_t(key_t &&) UVCXX_NOEXCEPT = default;

        key_t &operator=(key_t &&) UVCXX_NOEXCEPT = default;

        key_t() {
            UVCXX_APPLY_STRICT(uv_key_create(*this), "can not create key");
        }

        ~key_t() override {
            if (*this) uv_key_delete(*this);
        }

        void set(void *value) {
            uv_key_set(*this, value);
        }

        UVCXX_NODISCARD
        void *get() const {
            return uv_key_get(*this);
        }

        template<typename T>
        UVCXX_NODISCARD
        T *get() const {
            return (T *) uv_key_get(*this);
        }

        template<typename T>
        UVCXX_NODISCARD
        T &ref() const {
            return *((T *) uv_key_get(*this));
        }
    };
}

#endif //LIBUVCXX_KEY_H
