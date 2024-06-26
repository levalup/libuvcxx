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
#include <string>

#include "pencil_box.h"
#include "standard.h"
#include "../cxx/string.h"

namespace uvcxx {
    class assert_failed : public std::logic_error {
    public:
        using self = assert_failed;
        using supper = std::logic_error;


        explicit assert_failed(const uvcxx::string_view &cond)
                : supper(Message(cond)) {}

        template<typename... Args>
        assert_failed(uvcxx::string_view cond,
                      const std::string &file,
                      int line,
                      const Args &...args)
                : supper(Message(cond, file, line, args...)) {}

    public:
        static std::string Message(const uvcxx::string_view &cond) {
            std::ostringstream oss;
            oss << "assert failed: (" << cond << ")";
            return oss.str();
        }

        template<typename... Args>
        static std::string Message(uvcxx::string_view cond,
                                   const std::string &file,
                                   int line,
                                   const Args &...args) {
            std::string filename;
            {
                auto sep = file.rfind('/');
                if (sep != std::string::npos) {
                    filename = file.substr(sep + 1);
                } else {
                    sep = file.rfind('\\');
                    if (sep != std::string::npos) {
                        filename = file.substr(sep + 1);
                    } else {
                        filename = file;
                    }
                }
            }

            std::ostringstream oss;
            oss << "assert failed: (" << cond << ") at " << filename << ":" << line;
            if UVCXX_IF_CONSTEXPR (sizeof...(Args) > 0) {
                catout(oss, "; ", args...);
            }
            return oss.str();
        }
    };

    template<typename T, typename ...Args, typename std::enable_if<
            std::is_base_of<std::logic_error, T>::value, int>::type = 0>
    [[noreturn]]
    inline void throws(const Args &...args) {
        throw T(catstr(args...));
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
