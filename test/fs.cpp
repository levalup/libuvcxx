//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "uvcxx.h"

class fs_data_t : public uv::req_callback_t<uv_fs_t> {
};

int main() {
    uv::fs::open("a.txt", O_CREAT | O_RDONLY, 0777).then([&](int fd) {
        std::cout << "open fd = " << fd << std::endl;
        uv::fs::close(fd);
        // throw uvcxx::exception(UV_EAGAIN);
        throw std::logic_error("throw logic after close");
    }).except([](const std::exception &e) {
        auto xx = dynamic_cast<const uvcxx::exception*>(&e);
        if (xx != nullptr) {
            std::cerr << "errcode = " << xx->errcode() << std::endl;
        }
        std::cerr << e.what() << std::endl;
    });

    return uv::default_loop().run();
}
