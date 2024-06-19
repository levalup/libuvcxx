//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_BARRIER_H
#define LIBUVCXX_BARRIER_H

#include <functional>

#include <uv.h>

#include "uvcxx/cxx/except.h"
#include "uvcxx/inner/base.h"

namespace uv {
    class barrier_t : public uvcxx::pointer_raw_base_t<uv_barrier_t> {
    public:
        using self = barrier_t;
        using supper = uvcxx::pointer_raw_base_t<uv_barrier_t>;

        using supper::supper;

        barrier_t(barrier_t &&) = default;

        barrier_t &operator=(barrier_t &&) = default;

        explicit barrier_t(unsigned int count) {
            (void) uv_barrier_init(*this, count);
        }

        ~barrier_t() override {
            if (*this) uv_barrier_destroy(*this);
        }

        void wait() {
            uv_barrier_wait(*this);
        }
    };
}

#endif //LIBUVCXX_BARRIER_H
