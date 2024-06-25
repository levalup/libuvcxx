//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CXX_UTILS_H
#define LIBUVCXX_CXX_UTILS_H

#include <cstddef>
#include <functional>

#include <uv.h>

#include "except.h"

#define UVCXX_MAX_PATH 2048

namespace uvcxx {
    template<size_t Size>
    inline std::string try_get(const std::function<int(char *, size_t *)> &get) {
        char buffer[Size + 1];
        size_t size = sizeof(buffer);

        auto status = get(buffer, &size);
        if (status >= 0) return buffer;
        if (status == UV_ENOENT) return "";
        if (status != UV_ENOBUFS) UVCXX_THROW_OR_RETURN(status, "");

        auto data = new char[size];
        data[0] = '\0';

        status = get(data, &size);

        std::string result = data;
        delete[] data;

        if (status < 0) UVCXX_THROW_OR_RETURN(status, "");

        return result;
    }

    template<size_t Size, int Try = 4>
    inline std::string try_get(const std::function<int(char *, size_t)> &get) {
        char buffer[Size + 1];

        auto status = get(buffer, sizeof(buffer));
        if (status >= 0) return buffer;
        if (status == UV_ENOENT) return "";
        if (status != UV_ENOBUFS) UVCXX_THROW_OR_RETURN(status, "");

        auto size = Size;
        for (int i = 0; i < Try; ++i) {
            size *= 2;
            auto data = new char[size + 1];
            data[0] = '\0';

            status = get(data, size);

            std::string result = data;
            delete[] data;

            if (status >= 0) return result;
            if (status == UV_ENOENT) return "";
            if (status != UV_ENOBUFS) UVCXX_THROW_OR_RETURN(status, "");
        }

        UVCXX_THROW_OR_RETURN(status, "");
    }
}

#endif //LIBUVCXX_CXX_UTILS_H
