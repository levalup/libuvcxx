//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_RWLOCK_H
#define LIBUVCXX_RWLOCK_H

#include <functional>

#include <uv.h>

#include "uvcxx/inner/base.h"

namespace uv {
    class rwlock_t : public uvcxx::pointer_raw_base_t<uv_rwlock_t> {
    public:
        using self = rwlock_t;
        using supper = uvcxx::pointer_raw_base_t<uv_rwlock_t>;

        using supper::supper;

        rwlock_t(rwlock_t &&) UVCXX_NOEXCEPT = default;

        rwlock_t &operator=(rwlock_t &&) UVCXX_NOEXCEPT = default;

        rwlock_t() {
            (void) uv_rwlock_init(*this);
        }

        ~rwlock_t() override {
            if (*this) uv_rwlock_destroy(*this);
        }

        void rdlock() {
            uv_rwlock_rdlock(*this);
        }

        bool tryrdlock() {
            return uv_rwlock_tryrdlock(*this);
        }

        void rdunlock() {
            uv_rwlock_rdunlock(*this);
        }

        void wrlock() {
            uv_rwlock_wrlock(*this);
        }

        bool trywrlock() {
            return uv_rwlock_trywrlock(*this);
        }

        void wrunlock() {
            uv_rwlock_wrunlock(*this);
        }
    };
}

#endif //LIBUVCXX_RWLOCK_H
