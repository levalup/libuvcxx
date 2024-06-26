//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CXX_STRING_H
#define LIBUVCXX_CXX_STRING_H

#include <string>
#include <cstring>
#include <ostream>

namespace uvcxx {
    class string {
    public:
        const char *c_str = nullptr;

        string(std::nullptr_t) {};

        string(const std::string &str) : c_str(str.c_str()) {}

        string(const char *str) : c_str(str) {}

        operator const char *() const { return c_str; }
    };

    inline std::ostream &operator<<(std::ostream &out, const string &s) {
        return out << s.c_str;
    }

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
// std::string_view
#if __cpp_lib_string_view >= 201606L || __cplusplus >= 201703L || _MSC_VER >= 1910

        string_view(const std::string_view &str)
                : data(str.data()), size(str.size()) {}

#endif
    };

// std::string_view
#if __cpp_lib_string_view >= 201606L || __cplusplus >= 201703L || _MSC_VER >= 1910

    inline std::ostream &operator<<(std::ostream &out, const string_view &s) {
        return out << std::string_view(s.data, s.size);
    }

#else

    inline std::ostream &operator<<(std::ostream &out, const string_view &s) {
        return out << std::string(s.data, s.size);
    }

#endif
}

#endif //LIBUVCXX_CXX_STRING_H
