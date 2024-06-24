//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx/cxx/string.h"
#include "uvcxx/utils/assert.h"

std::string fs(uvcxx::string s) {
    std::cout << "|" << s.c_str << "|" << std::endl;
    return s.c_str;
}

std::string fsw(uvcxx::string_view s) {
    auto tmp = std::string(s.data, s.size);
    std::cout << "|" << tmp << "|" << std::endl;
    return tmp;
}

struct A {
    const char aa[4] = {'1', '2', '3', '4'};
    char bb[4] = {'1', '2', '3', '4'};
};

int main() {
    const char *c_str_1 = "c_str_1";
    const char c_str_2[] = "c_str_2";
    std::string cxx_str_1 = "cxx_str_1";

    uvcxx_assert(fs(c_str_1) == c_str_1);
    uvcxx_assert(fs(c_str_2) == c_str_2);
    uvcxx_assert(fs(cxx_str_1) == cxx_str_1);

    uvcxx_assert(fs("inplace c_str") == "inplace c_str");
    uvcxx_assert(fs(std::string("inplace cxx_str")) == "inplace cxx_str");
    uvcxx_assert(fs("temp " + std::string("cxx_str")) == "temp cxx_str");

    // uvcxx_assert(fsw(c_str_1) == c_str_1);
    uvcxx_assert(fsw(c_str_2) == c_str_2);
    uvcxx_assert(fsw(cxx_str_1) == cxx_str_1);

    uvcxx_assert(fsw("inplace c_str") == "inplace c_str");
    uvcxx_assert(fsw(std::string("inplace cxx_str")) == "inplace cxx_str");
    uvcxx_assert(fsw("temp " + std::string("cxx_str")) == "temp cxx_str");

    uvcxx_assert(fsw({"123456789", 3}) == "123");

    uvcxx_assert(fsw({A().aa, 4}) == "1234");   // use fsw(A().aa) raise problem
    uvcxx_assert(fsw({A().bb, 4}) == "1234");

#if __cplusplus >= 201703L || _MSC_VER >= 1910
    std::string_view sw_str = "sw_str_1";
    uvcxx_assert(fsw(sw_str) == sw_str);
#endif

    return 0;
}
