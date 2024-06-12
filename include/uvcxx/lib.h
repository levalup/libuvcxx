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

namespace uv {
    /**
     * This class is not thread-safe.
     */
    class lib_t {
    public:
        using self = lib_t;
        using raw_t = uv_lib_t;

        lib_t(const lib_t &) = delete;

        lib_t &operator=(const lib_t &) = delete;

        lib_t(lib_t &&that) noexcept {
            std::swap(m_lib, that.m_lib);
            std::swap(m_opened, that.m_opened);
        }

        lib_t &operator=(lib_t &&that) noexcept {
            std::swap(m_lib, that.m_lib);
            std::swap(m_opened, that.m_opened);
            return *this;
        }

        lib_t(std::nullptr_t) {}

        explicit lib_t(const char *filename) {
            auto err = uv_dlopen(filename, *this);
            if (err < 0) throw uvcxx::errcode(err);
            m_opened = true;
        }

        ~lib_t() {
            if (m_opened) {
                uv_dlclose(*this);
            }
        }

        explicit operator bool() const { return m_opened; }

        int open(const char *filename) {
            if (m_opened) {
                uv_dlclose(*this);
                m_opened = false;
            }
            auto err = uv_dlopen(filename, *this);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            m_opened = true;
            return err;
        }

        void close() {
            if (m_opened) {
                uv_dlclose(*this);
                m_opened = false;
            }
        }

        [[nodiscard]]
        const char *error() const {
            if (!m_opened) UVCXX_THROW_OR_RETURN(UV_EPERM, "");
            return uv_dlerror(*this);
        }

        int sym(const char *name, void **ptr) {
            if (!m_opened) UVCXX_THROW_OR_RETURN(UV_EPERM, UV_EPERM);
            auto err = uv_dlsym(*this, name, ptr);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);

            return err;
        }

        template<typename FUNC, typename std::enable_if_t<std::is_function_v<FUNC>, int> = 0>
        [[nodiscard]]
        FUNC *sym(const char *name) {
            FUNC *func;
            auto err = sym(name, &func);
            if (err < 0) return nullptr;
            return func;
        }

        operator raw_t *() { return &m_lib; }

        operator const raw_t *() const { return &m_lib; }

    private:
        uv_lib_t m_lib{};
        bool m_opened{false};
    };
}

#endif //LIBUVCXX_LIB_H
