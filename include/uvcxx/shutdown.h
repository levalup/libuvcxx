//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_SHUTDOWN_H
#define LIBUVCXX_SHUTDOWN_H

#include "req.h"

namespace uv {
    class stream_t;

    class shutdown_t : public inherit_req_t<uv_shutdown_t, req_t> {
    public:
        using self = shutdown_t;
        using supper = inherit_req_t<uv_shutdown_t, req_t>;

        [[nodiscard]]
        stream_t handle() const;

    public:
        class data_t : public req_callback_t<raw_t, int> {
        public:
            using self = data_t;
            using supper = req_callback_t<raw_t, int>;

            uvcxx::promise_cast<uvcxx::promise<>, raw_t *, int> promise;

            explicit data_t(const shutdown_t &req)
                    : supper(req), promise([](raw_t *, int) { return std::make_tuple(); }) {
            }

            uvcxx::promise_proxy<raw_t *, int> &proxy() noexcept override { return promise; }

            void finalize(raw_t *, int) noexcept override {};

            int check(raw_t *, int status) noexcept override { return status; }
        };
    };

    [[nodiscard]]
    inline uvcxx::promise<> shutdown(const shutdown_t &req, uv_stream_t *handle) {
        auto data = new shutdown_t::data_t(req);
        uvcxx::defer_delete delete_data(data);

        auto err = uv_shutdown(req, handle, shutdown_t::data_t::raw_callback);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

        delete_data.release();
        ((uv_shutdown_t *) req)->data = data;
        return data->promise.promise();
    }

    [[nodiscard]]
    inline uvcxx::promise<> shutdown(uv_stream_t *handle) {
        return shutdown({}, handle);
    }
}

#endif //LIBUVCXX_SHUTDOWN_H
