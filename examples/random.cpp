//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx/random.h"

int main() {
    int num = 0;
    for (int i = 0; i < 5; ++i) {
        (void) uv::random(nullptr, nullptr, &num, sizeof(num), 0, nullptr);
        std::cout << num << std::endl;
    }
    return 0;
}