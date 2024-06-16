//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_UTILITY_H
#define LIBUVCXX_UTILITY_H

#include "cxx/version.h"

namespace uv {
    /**
     * `libuv` provides miscellaneous utilities functions ,
     *     while `libuvcxx` focuses on simplifying the use of `handle` and `request`.
     * For this reason, additional encapsulation is not provided for the simple and straightforward functions.
     * If you need to use the related interfaces, please continue to utilize the `uv_*` versions in C language.
     * The implementation of `uv_random`-related interfaces can be found in `random.h`.
     */

#if UVCXX_SATISFY_VERSION(1, 34, 0)

    inline void sleep(unsigned int msec) { return uv_sleep(msec); }

#endif
}

#endif //LIBUVCXX_UTILITY_H
