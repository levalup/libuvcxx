//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_ERROR_H
#define LIBUVCXX_ERROR_H

#include <uv.h>

#include "uvcxx/cxx/version.h"

namespace uv {
    inline const char *strerror(int err) { return uv_strerror(err); }

#if UVCXX_SATISFY_VERSION(1, 22, 0)

    inline char *strerror_r(int err, char *buf, size_t buflen) { return uv_strerror_r(err, buf, buflen); }

#endif

    inline const char *err_name(int err) { return uv_err_name(err); }

#if UVCXX_SATISFY_VERSION(1, 22, 0)

    inline char *err_name_r(int err, char *buf, size_t buflen) { return uv_err_name_r(err, buf, buflen); }

#endif

#if UVCXX_SATISFY_VERSION(1, 10, 0)

    inline int translate_sys_error(int sys_errno) { return uv_translate_sys_error(sys_errno); }

#endif
}

#endif //LIBUVCXX_ERROR_H
