//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "uvcxx/idle.h"
#include "uvcxx/cxx/to_string.h"
#include "uvcxx/utils/assert.h"

int loop_handle_count(uv::loop_t &loop) {
    int count = 0;
    loop.walk([](uv_handle_t*, void* arg) {
        (*(int *)arg)++;
    }, &count);
    return count;
}

int main() {
    uv::loop_t loop;

    // case 1: init but not start
    {
        uv::idle_t idle(loop);
    }

    loop.run();
    uvcxx_assert(loop_handle_count(loop) == 0);

    // case 2: init, start
    int idle_times = 0;
    {
        uv::idle_t idle(loop);
        (void) idle.start().call([&]() mutable {
            ++idle_times;
            if (idle_times > 1) throw uvcxx::close_handle();
        });
    }

    loop.run(UV_RUN_ONCE);
    uvcxx_assert(idle_times == 1);
    uvcxx_assert(loop_handle_count(loop) == 1);

    // case 3: close handle by `uvcxx::close_handle`

    loop.run();
    uvcxx_assert(loop_handle_count(loop) == 0);

    // case 4: init, start and stop
    {
        uv::idle_t idle(loop);
        (void) idle.start().call([&]() mutable {
            idle.stop();
        });
        loop.run();
    }

    // close applied but event not handle
    uvcxx_assert(loop_handle_count(loop) == 1);
    loop.run(); // wait close event of case 4
    uvcxx_assert(loop_handle_count(loop) == 0);

    // good example

    {
        // close it in any time after not use it
        uv::idle_t idle(loop);
        (void) idle.start().call([&, idle]() mutable {
            ++idle_times;
            if (idle_times > 3) idle.close(nullptr);
        });
        // especially using temporary objects
        (void) uv::idle_t(loop).start().call([&]() mutable {
            ++idle_times;
            if (idle_times > 3) throw uvcxx::close_handle();
        });
    }
    loop.run();
    uvcxx_assert(idle_times > 3);
    uvcxx_assert(loop_handle_count(loop) == 0);

    // bad case.
    {
        // BAD: start but only stop, can not recycle the idle context data
        uv::idle_t idle(loop);
        (void) idle.start().call([idle]() mutable {
            idle.stop();
        });
    }
    loop.run();
    uvcxx_assert(loop_handle_count(loop) == 1);

    return 0;
}