//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_UDP_H
#define LIBUVCXX_UDP_H

#include "handle.h"
#include "udp_send.h"

namespace uv {
    class udp_t : public inherit_handle_t<uv_udp_t, handle_t> {
    public:
        using self = udp_t;
        using supper = inherit_handle_t<uv_udp_t, handle_t>;
        using raw_t = uv_udp_t;

        using supper::supper;

        class data_t : supper::data_t {
        public:
            using self = data_t;
            using supper = supper::data_t;

            uvcxx::callback_emitter<ssize_t, const uv_buf_t *, const sockaddr *, unsigned> recv_cb;
            uvcxx::callback_emitter<size_t, uv_buf_t *> alloc_cb;

            explicit data_t(udp_t &handle)
                    : supper(handle) {
                handle.watch(recv_cb);
                handle.watch(alloc_cb);
            }

            void close() noexcept override {
                alloc_cb.finalize();
                recv_cb.finalize();
                supper::close();
            }
        };

        udp_t() : self(default_loop()) {}

        udp_t(int flags) : self(default_loop(), flags) {}

        explicit udp_t(const loop_t &loop) {
            (void) uv_udp_init(loop, *this);
            // data will be deleted in close action
            set_data(new data_t(*this));
        }

        explicit udp_t(const loop_t &loop, int flags) {
            (void) uv_udp_init_ex(loop, *this, flags);
            // data will be deleted in close action
            set_data(new data_t(*this));
        }

        [[nodiscard]]
        size_t send_queue_size() const {
            return raw<raw_t>()->send_queue_size;
        }

        [[nodiscard]]
        size_t send_queue_count() const {
            return raw<raw_t>()->send_queue_count;
        }

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

        int open(uv_os_sock_t sock) {
            auto err = uv_udp_open(*this, sock);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int bind(const struct sockaddr *addr, unsigned int flags) {
            auto err = uv_udp_bind(*this, addr, flags);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int connect(const sockaddr *addr) {
            auto err = uv_udp_connect(*this, addr);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int getpeername(struct sockaddr *name, int *namelen) const {
            auto err = uv_udp_getpeername(*this, name, namelen);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int getsockname(struct sockaddr *name, int *namelen) const {
            auto err = uv_udp_getsockname(*this, name, namelen);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int set_membership(
                const char *multicast_addr, const char *interface_addr,
                uv_membership membership) const {
            auto err = uv_udp_set_membership(
                    *this, multicast_addr, interface_addr, membership);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int set_source_membership(
                const char *multicast_addr, const char *interface_addr, const char *source_addr,
                uv_membership membership) const {
            auto err = uv_udp_set_source_membership(
                    *this, multicast_addr, interface_addr, source_addr, membership);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int set_multicast_loop(bool on) {
            auto err = uv_udp_set_multicast_loop(*this, int(on));
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int set_multicast_ttl(int ttl) {
            auto err = uv_udp_set_multicast_ttl(*this, ttl);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int set_multicast_interface(const char *interface_addr) {
            auto err = uv_udp_set_multicast_interface(*this, interface_addr);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int set_broadcast(bool on) {
            auto err = uv_udp_set_broadcast(*this, int(on));
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int set_ttl(int ttl) {
            auto err = uv_udp_set_ttl(*this, ttl);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        [[nodiscard]]
        uvcxx::promise<> send(const udp_send_t &req, const uv_buf_t bufs[], unsigned int nbufs, const sockaddr *addr) {
            return uv::udp_send(req, *this, bufs, nbufs, addr);
        }

        [[nodiscard]]
        uvcxx::promise<> send(const uv_buf_t bufs[], unsigned int nbufs, const sockaddr *addr) {
            return uv::udp_send(*this, bufs, nbufs, addr);
        }

        int try_send(const uv_buf_t bufs[], unsigned int nbufs, const sockaddr *addr) {
            return uv_udp_try_send(*this, bufs, nbufs, addr);
        }


        [[nodiscard]]
        uvcxx::callback<size_t, uv_buf_t *> alloc() {
            auto data = get_data<data_t>();
            // memory alloc can not register multi-times callback
            return data->alloc_cb.callback().call(nullptr);
        }

        [[nodiscard]]
        uvcxx::callback<ssize_t, const uv_buf_t *, const sockaddr *, unsigned>
        recv_start() {
            auto err = uv_udp_recv_start(*this, raw_alloc_callback, raw_recv_callback);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            auto data = get_data<data_t>();
            return data->recv_cb.callback();
        }

        [[nodiscard]]
        bool using_recvmmsg() const {
            return uv_udp_using_recvmmsg(*this);
        }

        int recv_stop() {
            auto err = uv_udp_recv_stop(*this);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        [[nodiscard]]
        size_t get_send_queue_size() const {
            return uv_udp_get_send_queue_size(*this);
        }

        [[nodiscard]]
        size_t get_send_queue_count() const {
            return uv_udp_get_send_queue_count(*this);
        }

    public:
        static self borrow(raw_t *raw) {
            return self{std::shared_ptr<uv_handle_t>((uv_handle_t *) raw, [](uv_handle_t *) {})};
        }

    protected:
        explicit udp_t(std::shared_ptr<uv_handle_t> raw)
                : supper(std::move(raw)) {}

    private:
        static void raw_alloc_callback(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
            auto data = (data_t *) (handle->data);
            data->alloc_cb.emit(suggested_size, buf);
        }

        static void raw_recv_callback(
                raw_t *handle, ssize_t nread, const uv_buf_t *buf, const sockaddr *addr, unsigned flags) {
            auto data = (data_t *) (handle->data);
            data->recv_cb.emit(nread, buf, addr, flags);
        }
    };

    [[nodiscard]]
    inline udp_t udp_send_t::handle() const {
        return udp_t::borrow(raw<raw_t>()->handle);
    }
}

#endif //LIBUVCXX_UDP_H
