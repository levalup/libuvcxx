//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <thread>

#include "uvcxx/timer.h"
#include "uvcxx/utils/assert.h"

int main() {
    int v = 0;
    uv::timer_t().start(100, 100).call([&]() {
        ++v;
        throw uvcxx::close_handle();
    });
    auto backup = std::async([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return v;
    });
    auto status = uv::default_loop().run();
    uvcxx_assert(backup.get() > 0);
    return status;
}
