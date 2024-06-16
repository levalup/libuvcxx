//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>
#include <cstring>

#include "uvcxx/buf.h"

int main() {
    // create copyable shared buf
    uv::buf_t buf;
    buf.resize(16); //< allocate memory
    std::strcpy(buf.data(), "hello");
    buf.resize(8);  //< reuse the memory

    // data still store "hello"
    std::cout << buf.data() << " " << buf.size() << " " << buf.capacity();

    // convert to uv's C-structure
    uv_buf_t *uv_buf = buf;

    return 0;
}
