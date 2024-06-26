//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_TCP_H
#define LIBUVCXX_TCP_H

#include "connect.h"
#include "stream.h"

namespace uv {
    class tcp_t : public inherit_handle_t<uv_tcp_t, acceptable_stream_t> {
    public:
        using self = tcp_t;
        using supper = inherit_handle_t<uv_tcp_t, acceptable_stream_t>;

        tcp_t() : self(default_loop()) {}

        explicit tcp_t(const loop_t &loop) {
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_tcp_init(loop, *this);
            _attach_close_();
        }

#if UVCXX_SATISFY_VERSION(1, 7, 0)

        explicit tcp_t(int flags) : self(default_loop(), flags) {}

        explicit tcp_t(const loop_t &loop, int flags) {
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_tcp_init_ex(loop, *this, flags);
            _attach_close_();
        }

#endif

        int send_buffer_size(int *value) {
            auto err = uv_send_buffer_size(*this, value);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int recv_buffer_size(int *value) {
            auto err = uv_recv_buffer_size(*this, value);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int fileno(uv_os_fd_t *fd) const {
            auto err = uv_fileno(*this, fd);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        stream_t accept() override {
            self client(this->loop());
            uvcxx::defer close_client([&]() { client.close(nullptr); });

            auto err = uv_accept(*this, client);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);

            close_client.release();
            return client;
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

        UVCXX_NODISCARD
        uvcxx::promise<> connect(const connect_t &req, const sockaddr *addr) {
            return ::uv::tcp_connect(req, *this, addr);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect(const sockaddr *addr) {
            return ::uv::tcp_connect(*this, addr);
        }

#if UVCXX_SATISFY_VERSION(1, 32, 0)

        void close_reset(std::nullptr_t) {
            (void) close_for([&](void (*cb)(uv_handle_t *)) {
                (void) uv_tcp_close_reset(*this, cb);
            });
        }

        UVCXX_NODISCARD
        uvcxx::promise<> close_reset() {
            return close_for([&](void (*cb)(uv_handle_t *)) {
                (void) uv_tcp_close_reset(*this, cb);
            });
        }

#endif
    };

#if UVCXX_SATISFY_VERSION(1, 41, 0)

    UVCXX_NODISCARD
    inline int socketpair(int type, int protocol, uv_os_sock_t socket_vector[2], int flags0, int flags1) {
        auto err = uv_socketpair(type, protocol, socket_vector, flags0, flags1);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
        return err;
    }

#endif
}

#endif //LIBUVCXX_TCP_H
