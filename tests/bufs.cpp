//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx/cxx/buffer_like.h"
#include "uvcxx/buf.h"
#include "uvcxx/utils/assert.h"

int f(const uv_buf_t[], int) {
    std::cout << "uv_buf_t" << std::endl;
    return 0;
}

int f(uvcxx::buffer_like buf) {
    // std::cout << "uvcxx::buffer_like" << std::endl;
    std::cout << std::string((char *) buf.buf.base, buf.buf.len) << std::endl;
    return 1;
}

int f(std::initializer_list<uvcxx::buffer_like> bufs) {
    // std::cout << "std::initializer_list<uvcxx::buffer_like>" << std::endl;
    for (auto &buf: bufs) {
        f({buf.buf.base, buf.buf.len});
    }
    return 2;
}

struct A {
    int a;
};

struct B {
    std::vector<int> tt;
};

int main() {
    f("A");
    f({"123456", 3});
    uv::buf_t buf;
    buf.resize(12);
    buf.memset(0);
    buf.data()[0] = 'T';
    f(buf);
    f({"AA", "CC"});
    f({"string"});
    // f(std::string("tmp string"));
    // f({std::string("tmp string") + " 2"});
    // f({std::string("tmp string") + " 3", std::string("tmp string") + " 4"});

    int ii = 0x34353637;
    f(uvcxx::buffer_to(ii));

    return 0;
}