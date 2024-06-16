//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>
#include <fstream>

#include "uvcxx/fs_event.h"
#include "uvcxx/timer.h"
#include "uvcxx/cxx/to_string.h"

int main() {
    auto target = "a.txt";

    { std::ofstream touch(target); }

    uv::fs_event_t fs_event;
    fs_event.start(target, UV_FS_EVENT_RECURSIVE).call([](const char *filename, uv_fs_event events, int status) {
        if (status < 0) throw uvcxx::errcode(status);
        std::cout << filename << " " << uvcxx::to_string(events) << std::endl;
    });
    uv::timer_t().start(1 * 1000, 1).call([&]() {
        std::ofstream touch(target);
        throw uvcxx::close_handle();
    });
    uv::timer_t().start(2 * 1000, 1).call([&]() {
        fs_event.close(nullptr);
        throw uvcxx::close_handle();
    });
    // 2 seconds to close
    uv::default_loop().run();
    return 0;
}
