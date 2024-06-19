//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_SEM_H
#define LIBUVCXX_SEM_H

#include <functional>

#include <uv.h>

#include "uvcxx/cxx/except.h"
#include "uvcxx/inner/base.h"

namespace uv {
    class sem_t : public uvcxx::pointer_raw_base_t<uv_sem_t> {
    public:
        using self = sem_t;
        using supper = uvcxx::pointer_raw_base_t<uv_sem_t>;

        using supper::supper;

        sem_t(sem_t &&) = default;

        sem_t &operator=(sem_t &&) = default;

        explicit sem_t(unsigned int value) {
            (void) uv_sem_init(*this, value);
        }

        ~sem_t() override {
            if (*this) uv_sem_destroy(*this);
        }

        void post() {
            uv_sem_post(*this);
        }

        void wait() {
            uv_sem_wait(*this);
        }

        int trywait() {
            return uv_sem_trywait(*this);
        }
    };
}

#endif //LIBUVCXX_SEM_H
