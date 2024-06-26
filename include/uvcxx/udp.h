//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_UDP_H
#define LIBUVCXX_UDP_H

#include "cxx/string.h"

#include "handle.h"
#include "udp_send.h"

namespace uv {
    class udp_t : public inherit_handle_t<uv_udp_t, handle_t> {
    public:
        using self = udp_t;
        using supper = inherit_handle_t<uv_udp_t, handle_t>;

        using supper::supper;

        udp_t() : self(default_loop()) {}

        explicit udp_t(const loop_t &loop) {
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_udp_init(loop, *this);
            _attach_close_();
        }

#if UVCXX_SATISFY_VERSION(1, 7, 0)

        explicit udp_t(int flags) : self(default_loop(), flags) {}

        explicit udp_t(const loop_t &loop, int flags) {
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_udp_init_ex(loop, *this, flags);
            _attach_close_();
        }

#endif

        UVCXX_NODISCARD
        size_t send_queue_size() const {
            return raw<raw_t>()->send_queue_size;
        }

        UVCXX_NODISCARD
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

#if UVCXX_SATISFY_VERSION(1, 27, 0)

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

#endif

        int getsockname(struct sockaddr *name, int *namelen) const {
            auto err = uv_udp_getsockname(*this, name, namelen);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int set_membership(
                uvcxx::string multicast_addr, uvcxx::string interface_addr,
                uv_membership membership) const {
            auto err = uv_udp_set_membership(
                    *this, multicast_addr, interface_addr, membership);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

#if UVCXX_SATISFY_VERSION(1, 32, 0)

        int set_source_membership(
                uvcxx::string multicast_addr, uvcxx::string interface_addr, uvcxx::string source_addr,
                uv_membership membership) const {
            auto err = uv_udp_set_source_membership(
                    *this, multicast_addr, interface_addr, source_addr, membership);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

#endif

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

        int set_multicast_interface(uvcxx::string interface_addr) {
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

        UVCXX_NODISCARD
        uvcxx::promise<> send(const udp_send_t &req, const uv_buf_t bufs[], unsigned int nbufs, const sockaddr *addr) {
            return uv::udp_send(req, *this, bufs, nbufs, addr);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> send(const udp_send_t &req, uvcxx::buffer buf, const sockaddr *addr) {
            return send(req, &buf.buf, 1, addr);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> send(const udp_send_t &req, std::initializer_list<uvcxx::buffer> bufs, const sockaddr *addr) {
            std::vector<uv_buf_t> buffers;
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return send(req, buffers.data(), (unsigned int) buffers.size(), addr);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> send(const uv_buf_t bufs[], unsigned int nbufs, const sockaddr *addr) {
            return uv::udp_send(*this, bufs, nbufs, addr);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> send(uvcxx::buffer buf, const sockaddr *addr) {
            return send(&buf.buf, 1, addr);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> send(std::initializer_list<uvcxx::buffer> bufs, const sockaddr *addr) {
            std::vector<uv_buf_t> buffers;
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return send(buffers.data(), (unsigned int) buffers.size(), addr);
        }

        int try_send(const uv_buf_t bufs[], unsigned int nbufs, const sockaddr *addr) {
            return uv_udp_try_send(*this, bufs, nbufs, addr);
        }

        int try_send(uvcxx::mutable_buffer buf, const sockaddr *addr) {
            return try_send(&buf.buf, 1, addr);
        }

        int try_send(std::initializer_list<uvcxx::mutable_buffer> bufs, const sockaddr *addr) {
            std::vector<uv_buf_t> buffers;
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return try_send(buffers.data(), (unsigned int) buffers.size(), addr);
        }

        UVCXX_NODISCARD
        uvcxx::callback<size_t, uv_buf_t *> alloc() {
            // this alloc is not under Running state, so no `_detach_` applied.
            // memory alloc can not register multi-times callback
            return get_data<data_t>()->alloc_cb.callback().call(nullptr);
        }

        UVCXX_NODISCARD
        uvcxx::callback<ssize_t, const uv_buf_t *, const sockaddr *, unsigned>
        recv_start() {
            auto err = uv_udp_recv_start(*this, raw_alloc_callback, raw_recv_callback);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

            _detach_();
            return get_data<data_t>()->recv_cb.callback();
        }

#if UVCXX_SATISFY_VERSION(1, 39, 0)

        UVCXX_NODISCARD
        bool using_recvmmsg() const {
            return uv_udp_using_recvmmsg(*this);
        }

#endif

        int recv_stop() {
            auto err = uv_udp_recv_stop(*this);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            _attach_close_();
            return err;
        }

#if UVCXX_SATISFY_VERSION(1, 19, 0)

        UVCXX_NODISCARD
        size_t get_send_queue_size() const {
            return uv_udp_get_send_queue_size(*this);
        }

        UVCXX_NODISCARD
        size_t get_send_queue_count() const {
            return uv_udp_get_send_queue_count(*this);
        }

#endif

    public:
        static self borrow(raw_t *raw) {
            return self{borrow_t(raw)};
        }

    private:
        static void raw_alloc_callback(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
            auto data = (data_t * )(handle->data);
            data->alloc_cb.emit(suggested_size, buf);
        }

        static void raw_recv_callback(
                raw_t *handle, ssize_t nread, const uv_buf_t *buf, const sockaddr *addr, unsigned flags) {
            auto data = (data_t * )(handle->data);
            data->recv_cb.emit(nread, buf, addr, flags);
        }

        class data_t : supper::data_t {
        public:
            uvcxx::callback_emitter<ssize_t, const uv_buf_t *, const sockaddr *, unsigned> recv_cb;
            uvcxx::callback_emitter<size_t, uv_buf_t *> alloc_cb;

            explicit data_t(udp_t &handle)
                    : supper::data_t(handle) {
                handle.watch(recv_cb);
                handle.watch(alloc_cb);
            }

            ~data_t() override {
                alloc_cb.finalize();
                recv_cb.finalize();
            }
        };
    };

    UVCXX_NODISCARD
    inline udp_t udp_send_t::handle() const {
        return udp_t::borrow(raw<raw_t>()->handle);
    }
}

#endif //LIBUVCXX_UDP_H
