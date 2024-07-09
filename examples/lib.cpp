//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx/utils/assert.h"
#include "uvcxx/utils/platform.h"
#include "uvcxx/lib.h"

int main() {
#if UVCXX_OS_MAC
    auto libname = "libadd.dylib";
#elif UVCXX_OS_LINUX
    auto libname = "libadd.so";
#elif UVCXX_OS_WINDOWS
    auto libname = "add.dll";
#else
    auto libname = "add";
#endif

    uv::lib_t lib = nullptr;
    lib.open(libname);

    auto add = lib.sym<int(int, int)>("add");

    uvcxx_assert(add(1, 2) == 3);

    std::cout << "1 + 2 = " << add(1, 2) << std::endl;

    return 0;
}
