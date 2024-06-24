//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "uvcxx/fs.h"

int main() {
    std::cout << "----" << std::endl;

    uv::fs::open("a.txt", O_CREAT | O_WRONLY, 0777).then([&](int fd) {
        std::cout << "open fd = " << fd << std::endl;
        uv::fs::write(fd, "!!!!404!!!!", 0).then([fd](ssize_t) {
            (void) uv::fs::close(fd);
        });
        // throw uvcxx::exception(UV_EAGAIN);
        // throw std::logic_error("throw logic after close");
    }).except<uvcxx::errcode>([](const uvcxx::errcode &e) {
        std::cerr << "errcode = " << e.code() << std::endl;
    }).except<std::exception>([](const std::exception &e) {
        std::cerr << e.what() << std::endl;
    });

    {
        uv::fs_t tmp;
        uv::fs::mkdir(nullptr, tmp, "tmp", 0755, nullptr);
    }

    uv::fs::mkdtemp(std::string("tmp") + "/tfolder.XXXXXX").then([](const char *path) {
        std::cout << "mkdtemp " << path << std::endl;
        uv::fs_t tmp;
        uv::fs::rmdir(nullptr, tmp, path, nullptr);
    });

    return uv::default_loop().run();
}
