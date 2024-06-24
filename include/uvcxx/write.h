//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_WRITE_H
#define LIBUVCXX_WRITE_H

#include "req.h"

namespace uv {
    class stream_t;

    class write_t : public inherit_req_t<uv_write_t, req_t> {
    public:
        using self = write_t;
        using supper = inherit_req_t<uv_write_t, req_t>;

        [[nodiscard]]
        stream_t handle() const;

        [[nodiscard]]
        stream_t send_handle() const;

    public:
        class data_t : public req_callback_t<raw_t, int> {
        public:
            using self = data_t;
            using supper = req_callback_t<raw_t, int>;

            uvcxx::promise_cast<uvcxx::promise<>, raw_t *, int> promise;

            explicit data_t(const write_t &req)
                    : supper(req), promise([](raw_t *, int) { return std::make_tuple(); }) {
            }

            uvcxx::promise_proxy<raw_t *, int> &proxy() noexcept override { return promise; }

            void finalize(raw_t *, int) noexcept override {};

            int check(raw_t *, int status) noexcept override { return status; }
        };
    };

    [[nodiscard]]
    inline uvcxx::promise<> write(const write_t &req, uv_stream_t *handle, const uv_buf_t bufs[], unsigned int nbufs) {
        auto data = new write_t::data_t(req);
        uvcxx::defer_delete delete_data(data);

        auto err = uv_write(req, handle, bufs, nbufs, write_t::data_t::raw_callback);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

        delete_data.release();
        ((uv_write_t *) req)->data = data;
        return data->promise.promise();
    }

    [[nodiscard]]
    inline uvcxx::promise<> write(uv_stream_t *handle, const uv_buf_t bufs[], unsigned int nbufs) {
        return write({}, handle, bufs, nbufs);
    }

    [[nodiscard]]
    inline uvcxx::promise<> write2(
            const write_t &req,
            uv_stream_t *handle, const uv_buf_t bufs[], unsigned int nbufs, uv_stream_t *send_handle) {
        auto data = new write_t::data_t(req);
        uvcxx::defer_delete delete_data(data);

        auto err = uv_write2(req, handle, bufs, nbufs, send_handle, write_t::data_t::raw_callback);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

        delete_data.release();
        ((uv_write_t *) req)->data = data;
        return data->promise.promise();
    }

    [[nodiscard]]
    inline uvcxx::promise<> write2(
            uv_stream_t *handle, const uv_buf_t bufs[], unsigned int nbufs, uv_stream_t *send_handle) {
        return write2({}, handle, bufs, nbufs, send_handle);
    }
}

#endif //LIBUVCXX_WRITE_H
