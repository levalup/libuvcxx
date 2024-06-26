//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_LIB_H
#define LIBUVCXX_LIB_H

#include <atomic>
#include <cstddef>

#include <uv.h>

#include "cxx/except.h"
#include "cxx/string.h"
#include "inner/base.h"

namespace uv {
    /**
     * This class is not thread-safe.
     */
    class lib_t : uvcxx::extend_raw_base_t<uv_lib_t> {
    public:
        using self = lib_t;

        lib_t(const lib_t &) = delete;

        lib_t &operator=(const lib_t &) = delete;

        lib_t(lib_t &&that) UVCXX_NOEXCEPT = default;

        lib_t &operator=(lib_t &&that) UVCXX_NOEXCEPT = default;

        lib_t(std::nullptr_t) {}

        explicit lib_t(uvcxx::string filename) {
            auto err = uv_dlopen(filename, *this);
            if (err < 0) throw uvcxx::errcode(err, "can not open `", filename, "`");
            m_opened = true;
        }

        ~lib_t() {
            if (m_opened) {
                uv_dlclose(*this);
            }
        }

        explicit operator bool() const { return m_opened; }

        int open(uvcxx::string filename) {
            if (m_opened) {
                uv_dlclose(*this);
                m_opened = false;
            }
            auto err = uv_dlopen(filename, *this);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err, "can not open `", filename, "`");
            m_opened = true;
            return err;
        }

        void close() {
            if (m_opened) {
                uv_dlclose(*this);
                m_opened = false;
            }
        }

        UVCXX_NODISCARD
        const char *error() const {
            if (!m_opened) UVCXX_THROW_OR_RETURN(UV_EPERM, "", "cannot operate on a closed lib");
            return uv_dlerror(*this);
        }

        int sym(uvcxx::string name, void **ptr) const {
            if (!m_opened) UVCXX_THROW_OR_RETURN(UV_EPERM, UV_EPERM, "cannot operate on a closed lib");
            auto err = uv_dlsym(*this, name, ptr);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err, "cannot find symbol `", name, "`");

            return err;
        }

        template<typename FUNC, typename std::enable_if<std::is_function<FUNC>::value, int>::type = 0>
        UVCXX_NODISCARD
        FUNC *sym(uvcxx::string name) const {
            FUNC *func;
            auto err = sym(name, &func);
            if (err < 0) return nullptr;
            return func;
        }

    private:
        bool m_opened{false};
    };
}

#endif //LIBUVCXX_LIB_H
