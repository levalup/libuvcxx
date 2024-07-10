//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CXX_VERSION_H
#define LIBUVCXX_CXX_VERSION_H

#define UVCXX_VERSION_NUMBER(a, b, c) \
        ((a) * 1000000 + (b) * 1000 + (c))

#define UVCXX_SATISFY_VERSION(a, b, c) \
        (UVCXX_VERSION_NUMBER(UV_VERSION_MAJOR, UV_VERSION_MINOR, UV_VERSION_PATCH) \
        >= UVCXX_VERSION_NUMBER(a, b, c))

#endif //LIBUVCXX_CXX_VERSION_H
