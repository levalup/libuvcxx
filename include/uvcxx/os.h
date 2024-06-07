//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_OS_H
#define LIBUVCXX_OS_H

#include <uv.h>

namespace uv {
    inline uv_os_fd_t get_osfhandle(int fd) { return uv_get_osfhandle(fd); }

    inline int open_osfhandle(uv_os_fd_t os_fd) { return uv_open_osfhandle(os_fd); }
}

#endif //LIBUVCXX_OS_H
