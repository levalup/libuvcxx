//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_STRING_H
#define LIBUVCXX_STRING_H

#include <string>
#include <cstring>

namespace uvcxx {
    class string {
    public:
        const char *c_str = nullptr;

        string(std::nullptr_t) {};

        string(const std::string &str) : c_str(str.c_str()) {}

        string(const char *str) : c_str(str) {}

        operator const char *() const { return c_str; }
    };

    class string_view {
    public:
        const char *data = nullptr;
        size_t size = 0;

        string_view(std::nullptr_t) {};

        template<typename I, typename std::enable_if<std::is_integral<I>::value, int>::type = 0>
        string_view(const char *data, I size)
                : data(data), size(decltype(this->size)(size)) {}

        string_view(const std::string &str)
                : data(str.data()), size(str.size()) {}

        string_view(const char *str)
                : data(str), size(std::strlen(str)) {}

        template<int Size>
        string_view(const char (&str)[Size])
                : data(str), size(strnlen(str, Size)) {}

#if __cplusplus >= 201703L || _MSC_VER >= 1910

        string_view(const std::string_view &str)
                : data(str.data()), size(str.size()) {}

#endif
    };
}

#endif //LIBUVCXX_STRING_H
