//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CXX_WRAPPER_H
#define LIBUVCXX_CXX_WRAPPER_H

#include <cstddef>
#include <functional>

#include <uv.h>

#include "except.h"

#define UVCXX_APPLY(call, ret, ...) \
    do {                            \
        int status = call;          \
        if (status < 0) UVCXX_THROW_OR_RETURN(status, ret, ## __VA_ARGS__); \
    } while (false)

#define UVCXX_PROXY(call, ...) \
    do {                       \
        int status = call;     \
        if (status < 0) UVCXX_THROW_OR_RETURN(status, status, ## __VA_ARGS__); \
        return status;         \
    } while (false)

#define UVCXX_APPLY_STRICT(call, ...) \
    do {                              \
        int status = call;            \
        if (status < 0) throw ::uvcxx::errcode(status, ## __VA_ARGS__); \
    } while (false)

#define UVCXX_PROXY_STRICT(call, ...) \
    do {                              \
        int status = call;            \
        if (status < 0) throw ::uvcxx::errcode(status, ## __VA_ARGS__); \
        return status;                \
    } while (false)

#define UVCXX_MAX_PATH 2048

namespace uvcxx {
    template<size_t Size>
    inline std::string get_string(const std::function<int(char *, size_t *)> &get) {
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
    inline std::string get_string(const std::function<int(char *, size_t)> &get) {
        char buffer[Size + 1];

        auto status = get(buffer, sizeof(buffer));
        if (status >= 0) return buffer;
        if (status == UV_ENOENT) return "";
        if (status != UV_ENOBUFS) UVCXX_THROW_OR_RETURN(status, "");

        auto size = Size;
        for (decltype(Try) i = 0; i < Try; ++i) {
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

#endif //LIBUVCXX_CXX_WRAPPER_H
