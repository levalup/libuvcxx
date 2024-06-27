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
            UVCXX_PROXY(uv_send_buffer_size(*this, value));
        }

        int recv_buffer_size(int *value) {
            UVCXX_PROXY(uv_recv_buffer_size(*this, value));
        }

        int fileno(uv_os_fd_t *fd) const {
            UVCXX_PROXY(uv_fileno(*this, fd));
        }

        stream_t accept() override {
            self client(this->loop());

            UVCXX_APPLY(uv_accept(*this, client), nullptr);

            return client;
        }

        int open(uv_os_sock_t sock) {
            UVCXX_PROXY(uv_tcp_open(*this, sock));
        }

        int nodelay(bool enable) {
            UVCXX_PROXY(uv_tcp_nodelay(*this, int(enable)));
        }

        int keepalive(bool enable, unsigned int delay) {
            UVCXX_PROXY(uv_tcp_keepalive(*this, int(enable), delay));
        }

        int simultaneous_accepts(bool enable) {
            UVCXX_PROXY(uv_tcp_simultaneous_accepts(*this, int(enable)));
        }

        int bind(const struct sockaddr *addr, unsigned int flags) {
            UVCXX_PROXY(uv_tcp_bind(*this, addr, flags));
        }

        int getsockname(struct sockaddr *name, int *namelen) const {
            UVCXX_PROXY(uv_tcp_getsockname(*this, name, namelen));
        }

        int getpeername(struct sockaddr *name, int *namelen) const {
            UVCXX_PROXY(uv_tcp_getpeername(*this, name, namelen));
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect(const connect_t &req, const sockaddr *addr) {
            return tcp_connect(req, *this, addr);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect(const sockaddr *addr) {
            return tcp_connect(*this, addr);
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
        UVCXX_PROXY(uv_socketpair(type, protocol, socket_vector, flags0, flags1));
    }

#endif
}

#endif //LIBUVCXX_TCP_H
