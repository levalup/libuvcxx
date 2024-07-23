//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "uvcxx.h"

int main() {
    int v = 0;
    uv::idle_t().start().detach().call([&]() {
        ++v;
        throw uvcxx::close_handle();
    });
    auto status = uv::default_loop().run();
    uvcxx_assert(v == 1);
    return status;
}
