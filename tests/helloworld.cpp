//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx.h"

int main() {
    uv::loop_t loop; // = uv::default_loop();

    std::cout << "using default loop" << std::endl;
    loop.run();

    return 0;
}
