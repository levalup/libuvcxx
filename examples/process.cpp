//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#if defined(_MSC_VER)
#pragma warning(disable: 4996)
#endif

#include <iostream>
#include <cstdlib>

#include "uvcxx/process.h"
#include "uvcxx/utils/assert.h"

int main(int argc, const char *argv[]) {
    std::string main_env_a = "12";
    auto child_exit_code = 233;
    int64_t got_exit_code = 0;

    if (argc > 1) {
        std::cout << "<child> argv: ";
        for (int i = 0; i < argc; ++i) {
            std::cout << argv[i] << " ";
        }
        std::cout << std::endl;

        auto got_env_a = std::getenv("A");
        std::cout << "<child> env A = " << got_env_a << std::endl;
        uvcxx_assert(got_env_a == main_env_a);

        return child_exit_code;
    }

    std::cout << "<main> spawn child" << std::endl;

    uv::process_option_t me = uv::process_option_t(argv[0]);
    me.args({"a", "b", "c"})
            .flag(UV_INHERIT_STREAM)
            .env("A", main_env_a)
            .stdio(0, UV_IGNORE)
            .stdio(1, UV_INHERIT_FD, 1)
            .stdio(2, UV_INHERIT_FD, 2);
    // `close` will be implicitly called after the process exit.
    // once process closed before exit, the exit callback will never be reached.
    uv::process_t().spawn(me).then([&](int64_t exitcode, int signum) {
        std::cout << "<main> received child exit " << exitcode << " " << signum << std::endl;
        got_exit_code = exitcode;
    });

    uv::default_loop().run();

    uvcxx_assert(got_exit_code == child_exit_code);

    return 0;
}
