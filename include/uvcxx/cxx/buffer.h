//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CXX_BUFFER_H
#define LIBUVCXX_CXX_BUFFER_H

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
    class mutable_buffer {
    public:
        uv_buf_t buf{};

        mutable_buffer(std::nullptr_t) {
            buf.base = nullptr;
            buf.len = 0;
        }

        mutable_buffer(const uv_buf_t *buf) : buf(*buf) {}

        mutable_buffer(const uv_buf_t &buf) : buf(buf) {}

        template<typename I, typename std::enable_if<std::is_integral<I>::value, int>::type = 0>
        mutable_buffer(void *data, I size)
                : buf(init(data, size)) {}

        template<typename T, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
        mutable_buffer(std::vector<T> &vec)
                : buf(init(vec.data(), vec.size() * sizeof(T))) {}

        template<typename T, size_t Size, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
        mutable_buffer(std::array<T, Size> &arr)
                : buf(init(arr.data(), Size * sizeof(T))) {}

        template<typename T, size_t Size,
                typename std::enable_if<!std::is_const<T>::value && std::is_trivial<T>::value, int>::type = 0>
        mutable_buffer(T (&arr)[Size])
                : buf(init(arr, Size * sizeof(T))) {}

        mutable_buffer(uv::buf_t &buf);

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

    class buffer {
    public:
        uv_buf_t buf{};

        buffer(std::nullptr_t) {
            buf.base = nullptr;
            buf.len = 0;
        }

        buffer(const uv_buf_t *buf) : buf(*buf) {}

        buffer(const uv_buf_t &buf) : buf(buf) {}

        template<typename I, typename std::enable_if<std::is_integral<I>::value, int>::type = 0>
        buffer(const void *data, I size)
                : buf(init(data, size)) {}

        buffer(std::string &str)
                : buf(init(str.data(), str.size())) {}

        template<typename T, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
        buffer(std::vector<T> &vec)
                : buf(init(vec.data(), vec.size() * sizeof(T))) {}

        template<size_t Size>
        buffer(const char (&arr)[Size])
                : buf(init(arr, strnlen(arr, Size))) {}
// std::string_view
#if __cpp_lib_string_view >= 201606L || __cplusplus >= 201703L || _MSC_VER >= 1910

        buffer(std::string_view &str)
                : buf(init(str.data(), str.size())) {}

#endif

        buffer(const mutable_buffer &buf) : buf(buf.buf) {}

        buffer(uv::buf_t &buf);

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

    template<typename T, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
    inline mutable_buffer buffer_to(T &buf) {
        return {&buf, sizeof(T)};
    }

    template<typename T, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
    inline mutable_buffer buffer_to(T *buf) {
        return {buf, sizeof(T)};
    }

    template<typename T, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
    inline buffer buffer_to(const T &buf) {
        return {&buf, sizeof(T)};
    }

    template<typename T, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
    inline buffer buffer_to(const T *buf) {
        return {buf, sizeof(T)};
    }
}

#endif //LIBUVCXX_CXX_BUFFER_H
