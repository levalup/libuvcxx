//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_SHUTDOWN_H
#define LIBUVCXX_SHUTDOWN_H

#include "req.h"

namespace uv {
    class shutdown_t : public req_extend_t<uv_shutdown_t, req_t> {
    public:
        using self = shutdown_t;
        using supper = req_extend_t<uv_shutdown_t, req_t>;

        class data_t : public req_callback_t<raw_t, int> {
        public:
            using self = data_t;
            using supper = req_callback_t<raw_t, int>;

            uvcxx::promise_cast<uvcxx::promise<>, raw_t*, int> promise;

            explicit data_t(const shutdown_t &req)
                : supper(req), promise([](raw_t*, int){ return std::make_tuple(); }) {
            }

            uvcxx::promise_proxy<raw_t*, int> &proxy() noexcept override { return promise; }

            void finalize(raw_t*, int) noexcept override {};

            int check(raw_t*, int status) noexcept override { return status; }
        };

        shutdown_t() {
            set_data(new data_t(*this));
        }
    };

    [[nodiscard]]
    uvcxx::promise<> shutdown(const shutdown_t &req, uv_stream_t *handle) {
        auto err = uv_shutdown(req, handle, shutdown_t::data_t::raw_callback);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
        auto data = req.get_data< shutdown_t::data_t>();
        return data->promise.promise();
    }

    [[nodiscard]]
    uvcxx::promise<> shutdown(uv_stream_t *handle) {
        return shutdown({}, handle);
    }
}

#endif //LIBUVCXX_SHUTDOWN_H
