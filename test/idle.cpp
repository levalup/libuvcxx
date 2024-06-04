//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "uvcxx/idle.h"

int main() {
    uv::loop_t loop;
    int64_t counter = 0;
    uv::idle_t(loop).start().call([&](uv::idle_t *idle) {
        counter++;
        if (counter >= 1000) {
            idle->close().then([]() {
                std::cout << "closed" << std::endl;
            });
        }
        std::cout << counter << std::endl;
    });

    loop.run();

    return 0;
}