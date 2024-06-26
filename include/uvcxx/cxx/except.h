//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CXX_EXCEPT_H
#define LIBUVCXX_CXX_EXCEPT_H

#include <sstream>
#include <stdexcept>

#include <uv.h>

#include "../utils/standard.h"
#include "../utils/pencil_box.h"

namespace uvcxx {
    class exception : public std::runtime_error {
    public:
        using self = exception;
        using supper = std::runtime_error;

        using supper::supper;
    };

    class errcode : public exception {
    public:
        using self = errcode;
        using supper = exception;

        explicit errcode(int errcode)
                : supper(Message(errcode)), m_errcode(errcode) {}

        template<typename ...Args>
        explicit errcode(int errcode, const Args &...args)
                : supper(Message(errcode, args...)), m_errcode(errcode) {}

        template<typename I, typename std::enable_if<
                !std::is_same<int, I>::value && std::is_convertible<int, I>::value, int>::type = 0>
        explicit errcode(I errcode)
                : self(int(errcode)) {}

        template<typename I, typename ...Args, typename std::enable_if<
                !std::is_same<int, I>::value && std::is_convertible<int, I>::value, int>::type = 0>
        explicit errcode(I errcode, const Args &...args)
                : self(int(errcode), args...) {}

        static std::string Message(int errcode) {
            return catstr(uv_err_name(errcode), "(", errcode, "): ", uv_strerror(errcode));
        }

        template<typename T, typename ...Args>
        static std::string Message(int errcode, const T &arg, const Args &...args) {
            return catstr(uv_err_name(errcode), "(", errcode, "): ", uv_strerror(errcode), "; ", arg, args...);
        }

        UVCXX_NODISCARD
        int code() const { return m_errcode; }

        operator int() const { return m_errcode; }

        template<typename I, typename std::enable_if<std::is_convertible<int, I>::value, int>::type = 0>
        bool operator==(I v) { return m_errcode == int(v); }

        template<typename I, typename std::enable_if<std::is_convertible<int, I>::value, int>::type = 0>
        bool operator!=(I v) { return m_errcode != int(v); }

        template<typename I, typename std::enable_if<std::is_convertible<int, I>::value, int>::type = 0>
        bool operator<=(I v) { return m_errcode <= int(v); }

        template<typename I, typename std::enable_if<std::is_convertible<int, I>::value, int>::type = 0>
        bool operator>=(I v) { return m_errcode >= int(v); }

        template<typename I, typename std::enable_if<std::is_convertible<int, I>::value, int>::type = 0>
        bool operator<(I v) { return m_errcode < int(v); }

        template<typename I, typename std::enable_if<std::is_convertible<int, I>::value, int>::type = 0>
        bool operator>(I v) { return m_errcode > int(v); }

    private:
        int m_errcode = 0;
    };
}

#if defined(UVCXX_NO_EXCEPTION)
#define UVCXX_THROW(code, ...) return
#define UVCXX_THROW_OR_RETURN(code, ret, ...) return ret
#else
#define UVCXX_THROW(code, ...) throw ::uvcxx::errcode(code, ## __VA_ARGS__)
#define UVCXX_THROW_OR_RETURN(code, ret, ...) throw ::uvcxx::errcode(code, ## __VA_ARGS__)
#endif

#endif //LIBUVCXX_CXX_EXCEPT_H
