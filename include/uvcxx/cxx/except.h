//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_EXCEPT_H
#define LIBUVCXX_EXCEPT_H

#include <sstream>
#include <stdexcept>

#include <uv.h>

namespace uvcxx {
    class exception : public std::logic_error {
    public:
        using self = exception;
        using supper = std::logic_error;

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

        template<typename I, typename=typename std::enable_if_t<std::is_convertible_v<int, I>>>
        explicit errcode(I errcode)
                : self(int(errcode)) {}

        template<typename I, typename ...Args, typename=typename std::enable_if_t<std::is_convertible_v<int, I>>>
        explicit errcode(I errcode, const Args &...args)
                : self(int(errcode), args...) {}

        static std::string Message(int errcode) {
            std::ostringstream oss;
            Concat(oss, uv_err_name(errcode), "(", errcode, "): ", uv_strerror(errcode));
            return oss.str();
        }

        template<typename T, typename ...Args>
        static std::string Message(int errcode, const T &arg, const Args &...args) {
            std::ostringstream oss;
            Concat(oss, uv_err_name(errcode), "(", errcode, "): ", uv_strerror(errcode), "; ", arg, args...);
            return oss.str();
        }

        [[nodiscard]]
        int code() const { return m_errcode; }

        operator int() const { return m_errcode; }

        template<typename I, typename=typename std::enable_if_t<std::is_convertible_v<int, I>>>
        bool operator==(I v) { return m_errcode == int(v); }

        template<typename I, typename=typename std::enable_if_t<std::is_convertible_v<int, I>>>
        bool operator!=(I v) { return m_errcode != int(v); }

        template<typename I, typename=typename std::enable_if_t<std::is_convertible_v<int, I>>>
        bool operator<=(I v) { return m_errcode <= int(v); }

        template<typename I, typename=typename std::enable_if_t<std::is_convertible_v<int, I>>>
        bool operator>=(I v) { return m_errcode >= int(v); }

        template<typename I, typename=typename std::enable_if_t<std::is_convertible_v<int, I>>>
        bool operator<(I v) { return m_errcode < int(v); }

        template<typename I, typename=typename std::enable_if_t<std::is_convertible_v<int, I>>>
        bool operator>(I v) { return m_errcode > int(v); }

    private:
        int m_errcode = 0;

        static std::ostream &Concat(std::ostream &out) {
            return out;
        }

        template<typename T, typename ...Args>
        static std::ostream &Concat(std::ostream &out, const T &arg, const Args &...args) {
            return Concat(out << arg, args...);
        }
    };
}

#if defined(UVCXX_NO_EXCEPTION)
#define UVCXX_THROW(code, ...) return
#define UVCXX_THROW_OR_RETURN(code, ret, ...) return ret
#else
#define UVCXX_THROW(code, ...) throw ::uvcxx::errcode(code, ## __VA_ARGS__)
#define UVCXX_THROW_OR_RETURN(code, ret, ...) throw ::uvcxx::errcode(code, ## __VA_ARGS__)
#endif

#endif //LIBUVCXX_EXCEPT_H
