//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "uvcxx/fs.h"

int main() {
    uv::fs::open("a.txt", O_CREAT | O_RDONLY, 0777).then([&](int fd) {
        std::cout << "open fd = " << fd << std::endl;
        (void) uv::fs::close(fd);
        // throw uvcxx::exception(UV_EAGAIN);
        throw std::logic_error("throw logic after close");
    }).except<uvcxx::errcode>([](const uvcxx::errcode &e) {
        std::cerr << "errcode = " << e.code() << std::endl;
    }).except<std::exception>([](const std::exception &e) {
        std::cerr << e.what() << std::endl;
    });

    return uv::default_loop().run();
}
