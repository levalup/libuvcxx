//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CONNECT_H
#define LIBUVCXX_CONNECT_H

#include "req.h"

namespace uv {
    class stream_t;

    class connect_t : public req_extend_t<uv_connect_t, req_t> {
    public:
        using self = connect_t;
        using supper = req_extend_t<uv_connect_t, req_t>;

        class data_t : public req_callback_t<raw_t, int> {
        public:
            using self = data_t;
            using supper = req_callback_t<raw_t, int>;

            uvcxx::promise_cast<uvcxx::promise<>, raw_t *, int> promise;

            explicit data_t(const connect_t &req)
                    : supper(req), promise([](raw_t *, int) { return std::make_tuple(); }) {
            }

            uvcxx::promise_proxy<raw_t *, int> &proxy() noexcept override { return promise; }

            void finalize(raw_t *, int) noexcept override {};

            int check(raw_t *, int status) noexcept override { return status; }
        };

        [[nodiscard]]
        stream_t handle() const;
    };

    [[nodiscard]]
    inline uvcxx::promise<> tcp_connect(const connect_t &req, uv_tcp_t *handle, const sockaddr *addr) {
        auto data = new connect_t::data_t(req);
        uvcxx::defer_delete delete_data(data);

        auto err = uv_tcp_connect(req, handle, addr, connect_t::data_t::raw_callback);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

        delete_data.release();
        return data->promise.promise();
    }

    [[nodiscard]]
    inline uvcxx::promise<> tcp_connect(uv_tcp_t *handle, const sockaddr *addr) {
        return tcp_connect({}, handle, addr);
    }

    [[nodiscard]]
    inline uvcxx::promise<> pipe_connect(const connect_t &req, uv_pipe_t *handle, const char *name) {
        auto data = new connect_t::data_t(req);
        uvcxx::defer_delete delete_data(data);

        uv_pipe_connect(req, handle, name, connect_t::data_t::raw_callback);

        delete_data.release();
        return data->promise.promise();
    }

    [[nodiscard]]
    inline uvcxx::promise<> pipe_connect(uv_pipe_t *handle, const char *name) {
        return pipe_connect({}, handle, name);
    }

    [[nodiscard]]
    inline uvcxx::promise<> pipe_connect2(
            const connect_t &req, uv_pipe_t *handle, const char *name, size_t namelen, unsigned int flags) {
        auto data = new connect_t::data_t(req);
        uvcxx::defer_delete delete_data(data);

        auto err = uv_pipe_connect2(req, handle, name, namelen, flags, connect_t::data_t::raw_callback);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

        delete_data.release();
        return data->promise.promise();
    }

    [[nodiscard]]
    inline uvcxx::promise<> pipe_connect2(uv_pipe_t *handle, const char *name, size_t namelen, unsigned int flags) {
        return pipe_connect2({}, handle, name, namelen, flags);
    }
}

#endif //LIBUVCXX_CONNECT_H
