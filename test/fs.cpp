//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "uvcxx.h"

int main() {
    auto loop = uv::default_loop();

    uv::fs::open(loop, "a.txt", O_CREAT, 0).then([&](int fd) {
        uv::fs::close(loop, fd);
    });

    return loop.run();
}
