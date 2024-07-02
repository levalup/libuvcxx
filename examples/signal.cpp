//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx/signal.h"
#include "uvcxx/utils/assert.h"

int loop_handle_count(uv::loop_t &loop) {
    int count = 0;
    loop.walk([](uv_handle_t *, void *arg) {
        (*(int *) arg)++;
    }, &count);
    return count;
}

int main(int argc, const char *argv[]) {
    int sig = SIGINT;
    const char *key = "Ctrl+C";

    if (argc > 1 && argv[1] == std::string("ide")) {
        sig = SIGTERM;
        key = "[Stop]";
    }

    std::cout << "Test signal " << sig << " for " << key << std::endl;

    uv::loop_t loop;

    std::cout << "Press " << key << " at least " << 4 << " times." << std::endl;

    int count = 0;
    // hold handle to start and stop.
    {
        uv::signal_t signal(loop);
        signal.start(sig).call([&, key](int) mutable {
            ++count;
            std::cout << "Received " << key << " " << count << std::endl;
            signal.stop();
        });
        loop.run();
        (void) signal.start(sig);
        loop.run();
    }
    uvcxx_assert(loop_handle_count(loop) == 1, "count = ", loop_handle_count(loop));
    loop.run();
    uvcxx_assert(loop_handle_count(loop) == 0, "count = ", loop_handle_count(loop));
    // start backend to watch signal
    {
        // start backend
        uv::signal_t signal(loop);
        signal.start(sig).call([key, &count](int) mutable {
            ++count;
            std::cout << "Received " << key << " " << count << std::endl;
            throw uvcxx::close_handle();
        });
    }
    loop.run();
    uvcxx_assert(loop_handle_count(loop) == 0, "count = ", loop_handle_count(loop));
    // start oneshot
    {
        uv::signal_t(loop).start_oneshot(sig).then([key, &count](int) mutable {
            ++count;
            std::cout << "Received " << key << " " << count << std::endl;
        });
    }
    loop.run();
    uvcxx_assert(loop_handle_count(loop) == 0);

    return 0;
}
