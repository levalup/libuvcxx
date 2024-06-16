//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_MUTEX_H
#define LIBUVCXX_MUTEX_H

#include <functional>

#include <uv.h>

#include "uvcxx/inner/base.h"

namespace uv {
    class mutex_t : public uvcxx::pointer_raw_base_t<uv_mutex_t> {
    public:
        using self = mutex_t;
        using supper = uvcxx::pointer_raw_base_t<uv_mutex_t>;

        using supper::supper;

        mutex_t(mutex_t &&) = default;

        mutex_t &operator=(mutex_t &&) = default;

        mutex_t() {
            (void) uv_mutex_init(*this);
        }

        explicit mutex_t(bool recursive) {
            if (recursive) {
                (void) uv_mutex_init_recursive(*this);
            } else {
                (void) uv_mutex_init(*this);
            }
        }

        ~mutex_t() override {
            if (*this) uv_mutex_destroy(*this);
        }

        void lock() {
            uv_mutex_lock(*this);
        }

        bool try_lock() {
            return uv_mutex_trylock(*this);
        }

        void unlock() {
            uv_mutex_unlock(*this);
        }
    };

    class rwlock_t : public uvcxx::pointer_raw_base_t<uv_rwlock_t> {
    public:
        using self = rwlock_t;
        using supper = uvcxx::pointer_raw_base_t<uv_rwlock_t>;

        using supper::supper;

        rwlock_t(rwlock_t &&) = default;

        rwlock_t &operator=(rwlock_t &&) = default;

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

#endif //LIBUVCXX_MUTEX_H
