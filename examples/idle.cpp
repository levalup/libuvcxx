//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "uvcxx/idle.h"

int main() {
    uv::loop_t loop;
    int64_t counter = 0;
    uv::idle_t(loop).start().call([&]() {
        counter++;
        if (counter >= 1000) {
            throw uvcxx::close_handle();
        }
        std::cout << counter << std::endl;
    });

    loop.run();

    return 0;
}