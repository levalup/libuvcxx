//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_ONCE_H
#define LIBUVCXX_ONCE_H

#include <functional>

#include <uv.h>

#include "uvcxx/inner/base.h"

namespace uv {
    class once_t : public uvcxx::extend_raw_base_t<uv_once_t> {
    public:
        using self = once_t;
        using supper = uvcxx::extend_raw_base_t<uv_once_t>;

        once_t(const once_t &) = delete;

        once_t &operator=(const once_t &) = delete;

        once_t(once_t &&that) UVCXX_NOEXCEPT = delete;

        once_t &operator=(once_t &&that) UVCXX_NOEXCEPT = delete;

        once_t() : supper(UV_ONCE_INIT) {}

        void call(void (*callback)()) const {
            uv_once(*this, callback);
        }
    };
}

#endif //LIBUVCXX_ONCE_H
