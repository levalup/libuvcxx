//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx/version.h"

int main() {
    std::cout << uv::version_string() << std::endl;
    std::cout << std::hex << "0x" << uv::version() << std::endl;
    std::cout << std::hex << "0x" << UV_VERSION_HEX << std::endl;

    return 0;
}