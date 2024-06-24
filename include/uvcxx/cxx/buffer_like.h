//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_BUFFER_LIKE_H
#define LIBUVCXX_BUFFER_LIKE_H

#include <array>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

#include <uv.h>

namespace uv {
    class buf_t;
}

namespace uvcxx {
    class mutable_buffer_like {
    public:
        uv_buf_t buf{};

        mutable_buffer_like(std::nullptr_t) {
            buf.base = nullptr;
            buf.len = 0;
        }

        mutable_buffer_like(const uv_buf_t *buf) : buf(*buf) {}

        template<typename I, typename std::enable_if<std::is_integral<I>::value, int>::type = 0>
        mutable_buffer_like(void *data, I size)
                : buf(init(data, size)) {}

        template<typename T, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
        mutable_buffer_like(std::vector<T> &vec)
                : buf(init(vec.data(), vec.size() * sizeof(T))) {}

        template<typename T, size_t Size, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
        mutable_buffer_like(std::array<T, Size> &arr)
                : buf(init(arr.data(), Size * sizeof(T))) {}

        mutable_buffer_like(uv::buf_t &buf);

        operator uv_buf_t *() { return &buf; }

        operator const uv_buf_t *() const { return &buf; }

    private:
        template<typename I, typename std::enable_if<std::is_integral<I>::value, int>::type = 0>
        static uv_buf_t init(void *data, I size) {
            uv_buf_t buf{};
            buf.base = (decltype(buf.base)) (data);
            buf.len = (decltype(buf.len)) (size);
            return buf;
        }
    };

    class buffer_like {
    public:
        uv_buf_t buf{};

        buffer_like(std::nullptr_t) {
            buf.base = nullptr;
            buf.len = 0;
        }

        buffer_like(const uv_buf_t *buf) : buf(*buf) {}

        template<typename I, typename std::enable_if<std::is_integral<I>::value, int>::type = 0>
        buffer_like(const void *data, I size)
                : buf(init(data, size)) {}

        buffer_like(std::string &str)
                : buf(init(str.data(), str.size())) {}

        template<typename T, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
        buffer_like(std::vector<T> &vec)
                : buf(init(vec.data(), vec.size() * sizeof(T))) {}

        template<typename T, size_t Size, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
        buffer_like(std::array<T, Size> &arr)
                : buf(init(arr.data(), Size * sizeof(T))) {}

        template<size_t Size>
        buffer_like(const char (&arr)[Size])
                : buf(init(arr, strnlen(arr, Size))) {}

#if __cplusplus >= 201703L || _MSC_VER >= 1910

        buffer_like(std::string_view &str)
                : buf(init(str.data(), str.size())) {}

#endif

        buffer_like(const mutable_buffer_like &buf) : buf(buf.buf) {}

        buffer_like(uv::buf_t &buf);

        operator const uv_buf_t *() const { return &buf; }

    private:
        template<typename I, typename std::enable_if<std::is_integral<I>::value, int>::type = 0>
        static uv_buf_t init(const void *data, I size) {
            uv_buf_t buf{};
            buf.base = (decltype(buf.base)) (data);
            buf.len = (decltype(buf.len)) (size);
            return buf;
        }
    };

    template<typename T>
    inline mutable_buffer_like buffer_to(T &buf) {
        return {&buf, sizeof(T)};
    }

    template<typename T>
    inline mutable_buffer_like buffer_to(T *buf) {
        return {buf, sizeof(T)};
    }

    template<typename T>
    inline buffer_like buffer_to(const T &buf) {
        return {&buf, sizeof(T)};
    }

    template<typename T>
    inline buffer_like buffer_to(const T *buf) {
        return {buf, sizeof(T)};
    }
}

#endif //LIBUVCXX_BUFFER_LIKE_H
