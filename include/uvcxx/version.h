//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_VERSION_H
#define LIBUVCXX_VERSION_H

#include <uv.h>

#include "cxx/version.h"

#if !UVCXX_SATISFY_VERSION(1, 7, 0) && !defined(UV_VERSION_HEX)
#define UV_VERSION_HEX ((UV_VERSION_MAJOR << 16) | \
                        (UV_VERSION_MINOR <<  8) | \
                        (UV_VERSION_PATCH))
#endif


namespace uv {
    unsigned int version() { return uv_version(); }

    const char *version_string() { return uv_version_string(); }
}

#endif //LIBUVCXX_VERSION_H
