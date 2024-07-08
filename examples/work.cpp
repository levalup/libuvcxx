//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "uvcxx/os.h"
#include "uvcxx/work.h"
#include "uvcxx/utilities.h"

int main() {
    uv::os::setenv("UV_THREADPOOL_SIZE", "8");

    uv::loop_t loop;

    for (int i = 0; i < 10; ++i) {
        uv::queue_work(loop).then([i]() {
            uv::sleep(rand() % 10);
            std::cout << uvcxx::catstr("work-", i) << std::endl;
        });
    }

    return loop.run();
}
