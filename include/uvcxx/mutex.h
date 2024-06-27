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

        mutex_t(mutex_t &&) UVCXX_NOEXCEPT = default;

        mutex_t &operator=(mutex_t &&) UVCXX_NOEXCEPT = default;

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
}

#endif //LIBUVCXX_MUTEX_H
