//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CONNECT_H
#define LIBUVCXX_CONNECT_H

#include "req.h"

namespace uv {
    class stream_t;

    class connect_t : public inherit_req_t<uv_connect_t, req_t> {
    public:
        using self = connect_t;
        using supper = inherit_req_t<uv_connect_t, req_t>;

        UVCXX_NODISCARD
        inline stream_t handle() const;

    public:
        class callback_t : public req_callback_t<raw_t, int> {
        public:
            using self = callback_t;
            using supper = req_callback_t<raw_t, int>;

            uvcxx::promise_cast<uvcxx::promise<>, raw_t *, int> promise;

            explicit callback_t(const connect_t &req)
                    : supper(req), promise([](raw_t *, int) { return std::make_tuple(); }) {
            }

            uvcxx::promise_proxy<raw_t *, int> &proxy() UVCXX_NOEXCEPT override { return promise; }

            void finalize(raw_t *, int) UVCXX_NOEXCEPT override {};

            int check(raw_t *, int status) UVCXX_NOEXCEPT override { return status; }
        };
    };

    UVCXX_NODISCARD
    inline uvcxx::promise<> tcp_connect(const connect_t &req, uv_tcp_t *handle, const sockaddr *addr) {
        auto data = new connect_t::callback_t(req);
        uvcxx::defer_delete<connect_t::callback_t> delete_data(data);

        UVCXX_APPLY(uv_tcp_connect(req, handle, addr, connect_t::callback_t::raw_callback), nullptr);

        delete_data.release();
        ((uv_connect_t *) req)->data = data;
        return data->promise.promise();
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<> tcp_connect(uv_tcp_t *handle, const sockaddr *addr) {
        return tcp_connect({}, handle, addr);
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<> pipe_connect(const connect_t &req, uv_pipe_t *handle, const char *name) {
        auto data = new connect_t::callback_t(req);
        uvcxx::defer_delete<connect_t::callback_t> delete_data(data);

        uv_pipe_connect(req, handle, name, connect_t::callback_t::raw_callback);

        delete_data.release();
        ((uv_connect_t *) req)->data = data;
        return data->promise.promise();
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<> pipe_connect(uv_pipe_t *handle, const char *name) {
        return pipe_connect({}, handle, name);
    }

#if UVCXX_SATISFY_VERSION(1, 46, 0)

    UVCXX_NODISCARD
    inline uvcxx::promise<> pipe_connect2(
            const connect_t &req, uv_pipe_t *handle, const char *name, size_t namelen, unsigned int flags) {
        auto data = new connect_t::callback_t(req);
        uvcxx::defer_delete<connect_t::callback_t> delete_data(data);

        UVCXX_APPLY(uv_pipe_connect2(req, handle, name, namelen, flags, connect_t::callback_t::raw_callback), nullptr);

        delete_data.release();
        ((uv_connect_t *) req)->data = data;
        return data->promise.promise();
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<> pipe_connect2(uv_pipe_t *handle, const char *name, size_t namelen, unsigned int flags) {
        return pipe_connect2({}, handle, name, namelen, flags);
    }

#endif
}

#endif //LIBUVCXX_CONNECT_H
