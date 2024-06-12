//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_OS_H
#define LIBUVCXX_OS_H

#include <uv.h>

namespace uv {
    /**
     * `libuv` has many operating system-related interfaces,
     *     while `libuvcxx` is mainly designed for the convenient use of `handle` and `request`.
     * Therefore, no additional encapsulation is provided for simple and easy-to-use functions.
     * If you want to use related interfaces, please continue to use the `uv_*` version in C language.
     */
}

#endif //LIBUVCXX_OS_H
