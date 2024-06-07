//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_TCP_H
#define LIBUVCXX_TCP_H

#include "connect.h"
#include "stream.h"

namespace uv {
    class tcp_t : public handle_extend_t<uv_tcp_t, acceptable_stream_t> {
    public:
        using self = tcp_t;
        using supper = handle_extend_t<uv_tcp_t, acceptable_stream_t>;

        using raw_t = uv_tcp_t;

        tcp_t() : self(default_loop()) {
        }

        explicit tcp_t(const loop_t &loop) {
            (void) uv_tcp_init(loop, *this);
            // data will be deleted in close action
            set_data(new data_t(*this));
        }

        explicit tcp_t(const loop_t &loop, int flags) {
            (void) uv_tcp_init_ex(loop, *this, flags);
            // data will be deleted in close action
            set_data(new data_t(*this));
        }

        stream_t accept() override {
            self client(this->loop());
            uvcxx::defer close_client([&]() { client.close(nullptr); });

            auto err = uv_accept(*this, client);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);

            close_client.release();
            return client;
        }

        std::shared_ptr<acceptable_stream_t> accept_v2() override {
            auto client = std::make_shared<self>(this->loop());
            uvcxx::defer close_client([&]() { client->close(nullptr); });

            auto err = uv_accept(*this, *client);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);

            close_client.release();
            return client;
        }

        int accept(const stream_t &client) {
            auto err = uv_accept(*this, client);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int send_buffer_size(int *value) {
            auto err = uv_send_buffer_size(raw(), value);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int recv_buffer_size(int *value) {
            auto err = uv_recv_buffer_size(raw(), value);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int open(uv_os_sock_t sock) {
            auto err = uv_tcp_open(*this, sock);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int nodelay(bool enable) {
            auto err = uv_tcp_nodelay(*this, int(enable));
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int keepalive(bool enable, unsigned int delay) {
            auto err = uv_tcp_keepalive(*this, int(enable), delay);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int simultaneous_accepts(bool enable) {
            auto err = uv_tcp_simultaneous_accepts(*this, int(enable));
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int bind(const struct sockaddr *addr, unsigned int flags) {
            auto err = uv_tcp_bind(*this, addr, flags);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int getsockname(struct sockaddr *name, int *namelen) const {
            auto err = uv_tcp_getsockname(*this, name, namelen);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int getpeername(struct sockaddr *name, int *namelen) const {
            auto err = uv_tcp_getpeername(*this, name, namelen);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        [[nodiscard]]
        uvcxx::promise<> connect(const connect_t &req, const sockaddr *addr) {
            return ::uv::tcp_connect(req, *this, addr);
        }

        [[nodiscard]]
        uvcxx::promise<> connect(const sockaddr *addr) {
            return ::uv::tcp_connect(*this, addr);
        }
    };
}

#endif //LIBUVCXX_TCP_H
