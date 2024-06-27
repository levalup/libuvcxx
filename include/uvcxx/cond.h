//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_COND_H
#define LIBUVCXX_COND_H

#include <functional>

#include <uv.h>

#include "uvcxx/cxx/except.h"
#include "uvcxx/inner/base.h"

namespace uv {
    class cond_t : public uvcxx::pointer_raw_base_t<uv_cond_t> {
    public:
        using self = cond_t;
        using supper = uvcxx::pointer_raw_base_t<uv_cond_t>;

        using supper::supper;

        cond_t(cond_t &&) UVCXX_NOEXCEPT = default;

        cond_t &operator=(cond_t &&) UVCXX_NOEXCEPT = default;

        cond_t() {
            (void) uv_cond_init(*this);
        }

        ~cond_t() override {
            if (*this) uv_cond_destroy(*this);
        }

        void signal() {
            uv_cond_signal(*this);
        }

        void broadcast() {
            uv_cond_broadcast(*this);
        }

        void wait(uv_mutex_t *mutex) {
            uv_cond_wait(*this, mutex);
        }

        int timedwait(uv_mutex_t *mutex, uint64_t timeout) {
            return uv_cond_timedwait(*this, mutex, timeout);
        }
    };
}

#endif //LIBUVCXX_COND_H
