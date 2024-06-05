//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_BUF_H
#define LIBUVCXX_BUF_H

#include <cstdlib>
#include <memory>

#include <uv.h>

namespace uv {
    class buf_t {
    public:
        using self = buf_t;

        using raw_t = uv_buf_t;

        explicit buf_t(size_t size)
                : m_raw(make_buf(size)) {}

        char *data() { return m_raw->base; }

        [[nodiscard]]
        const char *data() const { return m_raw->base; }

        [[nodiscard]]
        size_t len() const { return size_t(m_raw->len); }

        operator raw_t *() const { return m_raw.get(); }

    private:
        std::shared_ptr<raw_t> m_raw;

        static std::shared_ptr<raw_t> make_buf(size_t size) {
            return {new_buf(size), delete_buf};
        }

        static void delete_buf(raw_t *buf) {
            std::free(buf);
        }

        static raw_t *new_buf(size_t size) {
            auto buf = (raw_t *) std::malloc(sizeof(raw_t));
            auto base = (char *) buf + sizeof(raw_t);

            // *buf = uv_buf_init(base, size);
            buf->base = base;
            buf->len = decltype(buf->len)(size);

            return buf;
        }
    };
}

#endif //LIBUVCXX_BUF_H
