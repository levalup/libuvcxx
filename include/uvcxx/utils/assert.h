//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_ASSERT_H
#define LIBUVCXX_ASSERT_H

#include <type_traits>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>

namespace uvcxx {
    class assert_failed : public std::logic_error {
    public:
        using self = assert_failed;
        using supper = std::logic_error;


        explicit assert_failed(const std::string_view &cond)
                : supper(Message(cond)) {}

        template<typename... Args>
        assert_failed(const std::string_view &cond,
                      const std::string_view &file,
                      int line,
                      const Args &...args)
                : supper(Message(cond, file, line, args...)) {}

    public:
        static std::string Message(const std::string_view &cond) {
            std::ostringstream oss;
            oss << "assert failed: (" << cond << ")";
            return oss.str();
        }

        template<typename... Args>
        static std::string Message(const std::string_view &cond,
                                   const std::string_view &file,
                                   int line,
                                   const Args &...args) {
            std::string_view filename;
            {
                auto sep = file.rfind('/');
                if (sep != std::string_view::npos) {
                    filename = file.substr(sep + 1);
                } else if (sep = file.rfind('\\'); sep != std::string_view::npos) {
                    filename = file.substr(sep + 1);
                } else {
                    filename = file;
                }
            }

            std::ostringstream oss;
            oss << "assert failed: (" << cond << ") at " << filename << ":" << line;
            if constexpr (sizeof...(Args) > 0) oss << "; ";
            (void) (oss << ... << args);
            return oss.str();
        }
    };

    template<typename T, typename ...Args, typename std::enable_if_t<
            std::is_base_of_v<std::logic_error, T>, int> = 0>
    [[noreturn]]
    inline void throws(const Args &...args) {
        std::ostringstream oss;
        (oss << ... << args);
        throw T(oss.str());
    }
}

#if defined(UVCXX_NO_ASSERT)
#define uvcxx_assert(cond, ...) do { ; } while (false)
#else
#define uvcxx_assert(cond, ...) \
    do {                        \
        if (!(cond))            \
            throw ::uvcxx::assert_failed(#cond, __FILE__, __LINE__, ## __VA_ARGS__); \
    } while (false)
#endif

#endif //LIBUVCXX_ASSERT_H
