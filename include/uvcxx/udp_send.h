//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_UDP_SEND_H
#define LIBUVCXX_UDP_SEND_H

#include "req.h"

namespace uv {
    class udp_t;

    class udp_send_t : public inherit_req_t<uv_udp_send_t, req_t> {
    public:
        using self = udp_send_t;
        using supper = inherit_req_t<uv_udp_send_t, req_t>;

        UVCXX_NODISCARD
        udp_t handle() const;

    public:
        class data_t : public req_callback_t<raw_t, int> {
        public:
            using self = data_t;
            using supper = req_callback_t<raw_t, int>;

            uvcxx::promise_cast<uvcxx::promise<>, raw_t *, int> promise;

            explicit data_t(const udp_send_t &req)
                    : supper(req), promise([](raw_t *, int) { return std::make_tuple(); }) {
            }

            uvcxx::promise_proxy<raw_t *, int> &proxy() UVCXX_NOEXCEPT override { return promise; }

            void finalize(raw_t *, int) UVCXX_NOEXCEPT override {};

            int check(raw_t *, int status) UVCXX_NOEXCEPT override { return status; }
        };
    };

    UVCXX_NODISCARD
    inline uvcxx::promise<> udp_send(
            const udp_send_t &req, uv_udp_t *handle,
            const uv_buf_t bufs[], unsigned int nbufs, const sockaddr *addr) {
        auto data = new udp_send_t::data_t(req);
        uvcxx::defer_delete<udp_send_t::data_t> delete_data(data);

        auto err = uv_udp_send(req, handle, bufs, nbufs, addr, udp_send_t::data_t::raw_callback);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

        delete_data.release();
        ((uv_udp_send_t *) req)->data = data;
        return data->promise.promise();
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<> udp_send(
            uv_udp_t *handle,
            const uv_buf_t bufs[], unsigned int nbufs, const sockaddr *addr) {
        return udp_send({}, handle, bufs, nbufs, addr);
    }
}

#endif //LIBUVCXX_UDP_SEND_H
