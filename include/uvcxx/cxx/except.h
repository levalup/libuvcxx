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

        template<typename I, typename=typename std::enable_if_t<std::is_integral_v<I>>>
        explicit errcode(I errcode, const std::string_view &msg = "") : self(int(errcode), msg) {}

        explicit errcode(int errcode, const std::string_view &msg = "")
                : supper(Message(errcode, msg)), m_errcode(errcode) {}

        static std::string Message(int errcode, const std::string_view &msg = "") {
            std::ostringstream oss;
            oss << uv_err_name(errcode);
            oss << "(" << errcode << "): ";
            oss << uv_strerror(errcode);
            if (!msg.empty()) {
                oss << "; " << msg;
            }
            return oss.str();
        }

        [[nodiscard]]
        int code() const { return m_errcode; }

        operator int() const { return m_errcode; }

    private:
        int m_errcode = 0;
    };
}

#if defined(UVCXX_NO_EXCEPTION)
#define UVCXX_THROW(code) return
#define UVCXX_THROW_OR_RETURN(code, ret) return ret
#else
#define UVCXX_THROW(code) throw ::uvcxx::errcode(code)
#define UVCXX_THROW_OR_RETURN(code, ret) throw ::uvcxx::errcode(code)
#endif

#endif //LIBUVCXX_EXCEPT_H
