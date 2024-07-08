//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "uvcxx/tty.h"
#include "uvcxx/utilities.h"
#include "uvcxx/cxx/to_string.h"

int main() {
    if (uv::guess_handle(1) != UV_TTY) {
        std::cout << "fd = 1 is " << uvcxx::to_string(uv::guess_handle(1)) << std::endl;
        return -1;
    }

    uv::loop_t loop;

    uv::tty_t tty(loop, 1);
    tty.set_mode(UV_TTY_MODE_NORMAL);

    tty.try_write("\033[31m"
                  "hello~"
                  "\033[0m");
    tty.close(nullptr);

    uv::tty::reset_mode();
    return loop.run();
}
