//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_UDP_H
#define LIBUVCXX_UDP_H

#include "cxx/buffer.h"
#include "cxx/string.h"
#include "utils/attached_promise.h"

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
            _initialized_();
        }

#if UVCXX_SATISFY_VERSION(1, 7, 0)

        explicit udp_t(int flags) : self(default_loop(), flags) {}

        explicit udp_t(const loop_t &loop, int flags) {
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_udp_init_ex(loop, *this, flags);
            _initialized_();
        }

#endif

        self &detach() {
            _detach_();
            return *this;
        }

        UVCXX_NODISCARD
        size_t send_queue_size() const {
            return raw<raw_t>()->send_queue_size;
        }

        UVCXX_NODISCARD
        size_t send_queue_count() const {
            return raw<raw_t>()->send_queue_count;
        }

        int send_buffer_size(int *value) {
            UVCXX_PROXY(uv_send_buffer_size(*this, value));
        }

        int recv_buffer_size(int *value) {
            UVCXX_PROXY(uv_recv_buffer_size(*this, value));
        }

        int fileno(uv_os_fd_t *fd) const {
            UVCXX_PROXY(uv_fileno(*this, fd));
        }

        int open(uv_os_sock_t sock) {
            UVCXX_PROXY(uv_udp_open(*this, sock));
        }

        int bind(const struct sockaddr *addr, unsigned int flags) {
            UVCXX_PROXY(uv_udp_bind(*this, addr, flags));
        }

#if UVCXX_SATISFY_VERSION(1, 27, 0)

        int connect(const sockaddr *addr) {
            UVCXX_PROXY(uv_udp_connect(*this, addr));
        }

        int getpeername(sockaddr *name, int *namelen) const {
            UVCXX_PROXY(uv_udp_getpeername(*this, name, namelen));
        }

#endif

        int getsockname(sockaddr *name, int *namelen) const {
            UVCXX_PROXY(uv_udp_getsockname(*this, name, namelen));
        }

        int set_membership(
                uvcxx::string multicast_addr, uvcxx::string interface_addr,
                uv_membership membership) const {
            UVCXX_PROXY(uv_udp_set_membership(
                    *this, multicast_addr, interface_addr, membership));
        }

#if UVCXX_SATISFY_VERSION(1, 32, 0)

        int set_source_membership(
                uvcxx::string multicast_addr, uvcxx::string interface_addr, uvcxx::string source_addr,
                uv_membership membership) const {
            UVCXX_PROXY(uv_udp_set_source_membership(
                    *this, multicast_addr, interface_addr, source_addr, membership));
        }

#endif

        int set_multicast_loop(bool on) {
            UVCXX_PROXY(uv_udp_set_multicast_loop(*this, int(on)));
        }

        int set_multicast_ttl(int ttl) {
            UVCXX_PROXY(uv_udp_set_multicast_ttl(*this, ttl));
        }

        int set_multicast_interface(uvcxx::string interface_addr) {
            UVCXX_PROXY(uv_udp_set_multicast_interface(*this, interface_addr));
        }

        int set_broadcast(bool on) {
            UVCXX_PROXY(uv_udp_set_broadcast(*this, int(on)));
        }

        int set_ttl(int ttl) {
            UVCXX_PROXY(uv_udp_set_ttl(*this, ttl));
        }

        UVCXX_NODISCARD
        uvcxx::attached_promise<> send(
                const udp_send_t &req, const uv_buf_t bufs[], unsigned int nbufs, const sockaddr *addr = nullptr) {
            return {*this, udp_send(req, *this, bufs, nbufs, addr)};
        }

        UVCXX_NODISCARD
        uvcxx::attached_promise<> send(const udp_send_t &req, uvcxx::buffer buf, const sockaddr *addr = nullptr) {
            return this->send(req, &buf.buf, 1, addr);
        }

        UVCXX_NODISCARD
        uvcxx::attached_promise<> send(
                const udp_send_t &req, std::initializer_list<uvcxx::buffer> bufs, const sockaddr *addr = nullptr) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return this->send(req, buffers.data(), (unsigned int) buffers.size(), addr);
        }

        UVCXX_NODISCARD
        uvcxx::attached_promise<> send(const uv_buf_t bufs[], unsigned int nbufs, const sockaddr *addr = nullptr) {
            return this->send({}, bufs, nbufs, addr);
        }

        UVCXX_NODISCARD
        uvcxx::attached_promise<> send(uvcxx::buffer buf, const sockaddr *addr = nullptr) {
            return this->send(&buf.buf, 1, addr);
        }

        UVCXX_NODISCARD
        uvcxx::attached_promise<> send(std::initializer_list<uvcxx::buffer> bufs, const sockaddr *addr = nullptr) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return this->send(buffers.data(), (unsigned int) buffers.size(), addr);
        }

        int try_send(const uv_buf_t bufs[], unsigned int nbufs, const sockaddr *addr = nullptr) {
            return uv_udp_try_send(*this, bufs, nbufs, addr);
        }

        int try_send(uvcxx::buffer buf, const sockaddr *addr = nullptr) {
            return this->try_send(&buf.buf, 1, addr);
        }

        int try_send(std::initializer_list<uvcxx::buffer> bufs, const sockaddr *addr = nullptr) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return this->try_send(buffers.data(), (unsigned int) buffers.size(), addr);
        }

        UVCXX_NODISCARD
        uvcxx::callback<size_t, uv_buf_t *> alloc_callback() {
            return get_data<data_t>()->alloc_cb.callback();
        }

        /**
         * Different from `alloc_callback` which only acquires the callback function,
         *     this method will clear the previously registered callback to avoid repeatedly `allocate`.
         * @return
         */
        UVCXX_NODISCARD
        uvcxx::callback<size_t, uv_buf_t *> alloc() {
            // this alloc is not under Running state, so no `_detach_` applied.
            // memory alloc can not register multi-times callback
            // use call(nullptr) to avoid call alloc multi times.
            return get_data<data_t>()->alloc_cb.callback().call(nullptr);
        }

        UVCXX_NODISCARD
        uvcxx::callback<ssize_t, const uv_buf_t *, const sockaddr *, uv_udp_flags> recv_callback() {
            return get_data<data_t>()->recv_cb.callback();
        }

        UVCXX_NODISCARD
        uvcxx::attached_callback<ssize_t, const uv_buf_t *, const sockaddr *, uv_udp_flags>
        recv_start() {
            UVCXX_APPLY(uv_udp_recv_start(*this, raw_alloc_callback, raw_recv_callback), nullptr);
            _detach_();
            return {*this, recv_callback()};
        }

#if UVCXX_SATISFY_VERSION(1, 39, 0)

        UVCXX_NODISCARD
        bool using_recvmmsg() const {
            return uv_udp_using_recvmmsg(*this);
        }

#endif

        int recv_stop() {
            UVCXX_PROXY(uv_udp_recv_stop(*this));
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
        static inline self borrow(raw_t *raw) {
            return self{borrow_t(raw)};
        }

    private:
        static void raw_alloc_callback(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
            auto data = (data_t * )(handle->data);
            data->alloc_cb.emit(suggested_size, buf);
        }

        static void raw_recv_callback(
                raw_t *handle, ssize_t nread, const uv_buf_t *buf, const sockaddr *addr, unsigned int flags) {
            auto data = (data_t * )(handle->data);
            data->recv_cb.emit(nread, buf, addr, uv_udp_flags(flags));
        }

        class data_t : supper::data_t {
        public:
            uvcxx::callback_emitter<ssize_t, const uv_buf_t *, const sockaddr *, uv_udp_flags> recv_cb;
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
