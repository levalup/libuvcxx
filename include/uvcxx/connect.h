//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CONNECT_H
#define LIBUVCXX_CONNECT_H

#include "req.h"

namespace uv {
    class connect_t : public req_extend_t<uv_connect_t, req_t> {
    public:
        using self = connect_t;
        using supper = req_extend_t<uv_connect_t, req_t>;

        class data_t : public req_callback_t<raw_t, int> {
        public:
            using self = data_t;
            using supper = req_callback_t<raw_t, int>;

            uvcxx::promise_cast<uvcxx::promise<>, raw_t*, int> promise;

            explicit data_t(const connect_t &req)
                : supper(req), promise([](raw_t*, int){ return std::make_tuple(); }) {
            }

            uvcxx::promise_proxy<raw_t*, int> &proxy() noexcept override { return promise; }

            void finalize(raw_t*, int) noexcept override {};

            int check(raw_t*, int status) noexcept override { return status; }
        };

        connect_t() {
            set_data(new data_t(*this));
        }
    };

    [[nodiscard]]
    uvcxx::promise<> tcp_connect(const connect_t &req, uv_tcp_t *handle, const sockaddr* addr) {
        auto err = uv_tcp_connect(req, handle, addr, connect_t::data_t::raw_callback);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
        auto data = req.get_data< connect_t::data_t>();
        return data->promise.promise();
    }

    [[nodiscard]]
    uvcxx::promise<> tcp_connect(uv_tcp_t *handle, const sockaddr* addr) {
        return tcp_connect({}, handle, addr);
    }
}

#endif //LIBUVCXX_CONNECT_H
