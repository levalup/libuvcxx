//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx/thread.h"

int main() {
    // create thread with work callback function
    uv::thread_t t([&]() {
        for (int i = 0; i < 10; ++i) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    });

    // Explicit call to `join`.
    // It will be implicitly called during the destruction.
    t.join();
    // Repeated calls are also acceptable.
    t.join();

    return 0;
}
