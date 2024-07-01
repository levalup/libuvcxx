//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "uvcxx.h"

int main() {
    uv::loop_t loop;

    int before = 0;
    int after = 0;
    int idle_times = 0;
    {
        uvcxx::ref<uv::idle_t> idle = uv::idle_t(loop);
        idle->start().call([&]() {
            ++idle_times;
            throw uvcxx::close_handle();
        });

        loop.walk([](uv_handle_t *handle, void *arg) {
            (*(int *) arg)++;
            std::cout << uvcxx::to_string(handle->type) << " " << handle << std::endl;
        }, &before);
    }

    loop.run();

    loop.walk([](uv_handle_t *handle, void *arg) {
        (*(int *) arg)++;
        std::cout << uvcxx::to_string(handle->type) << " " << handle << std::endl;
    }, &after);

    uvcxx_assert(before == 1 && after == 0 && idle_times == 0);

    return 0;
}
