//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "uvcxx/fs.h"
#include "uvcxx/buf.h"
#include "uvcxx/cxx/to_string.h"

int main() {
    std::string content = "!!!!404!!!!";

    // create a.txt and write content into it.
    uv::fs::open("a.txt", UV_FS_O_CREAT | UV_FS_O_WRONLY, 0777).then([&](int fd) {
        std::cout << "open fd = " << fd << std::endl;
        uv::fs::write(fd, content, 0).then([](ssize_t size) {
            std::cout << "write size " << size << std::endl;
        }).finally([fd]() {
            // close file in finally, whether write succeed or not.
            (void) uv::fs::close(fd);
        });
    }).except<uvcxx::errcode>([](const uvcxx::errcode &e) {
        std::cerr << "errcode = " << e.code() << std::endl;
        exit(-1);
    });

    uv::default_loop().run();

    // read a.txt content
    uv::fs::open("a.txt", UV_FS_O_RDONLY).then([&](int fd) {
        uv::buf_t buf(1024);
        uv::fs::read(fd, buf, 0).then([buf](ssize_t size) {
            std::cout << "read size " << size << std::endl;
            std::cout << "read content: " << std::string(buf.data(), size) << std::endl;
        }).finally([fd]() {
            // close file in finally, whether write succeed or not.
            (void) uv::fs::close(fd);
        });
    });

    uv::default_loop().run();

    {
        // call sync version mkdir
        uv::fs_t tmp;
        uv::fs::mkdir(nullptr, tmp, "tmp", 0755, nullptr);
    }

    // make temp folder
    uv::fs::mkdtemp(std::string("tmp") + "/folder.XXXXXX").then([](const char *path) {
        std::cout << "mkdtemp " << path << std::endl;
        uv::fs_t tmp;
        uv::fs::rmdir(nullptr, tmp, path, nullptr);
    });

#if UVCXX_SATISFY_VERSION(1, 28, 0)

    // readdir
    uv::fs::opendir("tmp").then([](uv_dir_t *dir) {
        uv::fs::readdir(dir, 1024).then([](uv_dirent_t *file, size_t size) {
            for (decltype(size) i = 0; i < size; ++i) {
                std::cout << "<" << uvcxx::to_string(file[i].type) << "> " << file[i].name << std::endl;
            }
        }).finally([dir]() {
            (void) uv::fs::closedir(dir);
        });
    });

#endif

    // scandir
    uv::fs::scandir("tmp", 0).then([](const uv::fs::scan_next &next) {
        uv_dirent_t file{};
        while (next(&file) >= 0) {
            std::cout << "<" << uvcxx::to_string(file.type) << "> " << file.name << std::endl;
        }
    });

    return uv::default_loop().run();
}
