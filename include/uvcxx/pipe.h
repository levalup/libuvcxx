//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_PIPE_H
#define LIBUVCXX_PIPE_H

#include "cxx/string.h"

#include "connect.h"
#include "stream.h"

namespace uv {
    class pipe_t : public inherit_handle_t<uv_pipe_t, acceptable_stream_t> {
    public:
        using self = pipe_t;
        using supper = inherit_handle_t<uv_pipe_t, acceptable_stream_t>;

        using raw_t = uv_pipe_t;

        explicit pipe_t(bool ipc) : self(default_loop(), ipc) {}

        explicit pipe_t(const loop_t &loop, bool ipc) {
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_pipe_init(loop, *this, int(ipc));
            _attach_close_();
        }

        UVCXX_NODISCARD
        bool ipc() const {
            return raw<raw_t>()->ipc;
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

        stream_t accept() override {
            self client(this->loop());

            UVCXX_APPLY(uv_accept(*this, client), nullptr);

            return client;
        }

        int open(uv_file file) {
            UVCXX_PROXY(uv_pipe_open(*this, file));
        }

        int bind(uvcxx::string name) {
            UVCXX_PROXY(uv_pipe_bind(*this, name));
        }

#if UVCXX_SATISFY_VERSION(1, 46, 0)

        int bind2(const char *name, size_t namelen, unsigned int flags) {
            UVCXX_PROXY(uv_pipe_bind2(*this, name, namelen, flags));
        }

        int bind2(uvcxx::string_view name, unsigned int flags) {
            return bind2(name.data, name.size, flags);
        }

        int bind(uvcxx::string_view name, unsigned int flags) {
            return bind2(name.data, name.size, flags);
        }

#endif

        UVCXX_NODISCARD
        uvcxx::promise<> connect(const connect_t &req, uvcxx::string name) {
            return pipe_connect(req, *this, name);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect(uvcxx::string name) {
            return pipe_connect(*this, name);
        }

#if UVCXX_SATISFY_VERSION(1, 46, 0)

        UVCXX_NODISCARD
        uvcxx::promise<> connect2(const connect_t &req, const char *name, size_t namelen, unsigned int flags) {
            return pipe_connect2(req, *this, name, namelen, flags);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect2(const char *name, size_t namelen, unsigned int flags) {
            return pipe_connect2(*this, name, namelen, flags);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect2(const connect_t &req, uvcxx::string_view name, unsigned int flags) {
            return pipe_connect2(req, *this, name.data, name.size, flags);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect2(uvcxx::string_view name, unsigned int flags) {
            return pipe_connect2(*this, name.data, name.size, flags);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect(const connect_t &req, uvcxx::string_view name, unsigned int flags) {
            return pipe_connect2(req, *this, name.data, name.size, flags);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect(uvcxx::string_view name, unsigned int flags) {
            return pipe_connect2(*this, name.data, name.size, flags);
        }

#endif

        int getsockname(char *buffer, size_t *size) const {
            return uv_pipe_getsockname(*this, buffer, size);
        }

        UVCXX_NODISCARD
        std::string getsockname() const {
            uv_pipe_t *handle = *this;
            return uvcxx::get_string<256>([handle](char *buffer, size_t *size) {
                return uv_pipe_getsockname(handle, buffer, size);
            });
        }

#if UVCXX_SATISFY_VERSION(1, 3, 0)

        int getpeername(char *buffer, size_t *size) const {
            return uv_pipe_getpeername(*this, buffer, size);
        }

        UVCXX_NODISCARD
        std::string getpeername() const {
            uv_pipe_t *handle = *this;
            return uvcxx::get_string<256>([handle](char *buffer, size_t *size) {
                return uv_pipe_getpeername(handle, buffer, size);
            });
        }

#endif

        void pending_instances(int count) const {
            uv_pipe_pending_instances(*this, count);
        }

        UVCXX_NODISCARD
        int pending_count() {
            return uv_pipe_pending_count(*this);
        }

        UVCXX_NODISCARD
        uv_handle_type pending_type() {
            return uv_pipe_pending_type(*this);
        }

#if UVCXX_SATISFY_VERSION(1, 16, 0)

        int chmod(int flags) {
            UVCXX_PROXY(uv_pipe_chmod(*this, flags));
        }

#endif
    };

#if UVCXX_SATISFY_VERSION(1, 41, 0)

    inline int pipe(uv_file fds[2], int read_flags, int write_flags) {
        UVCXX_PROXY(uv_pipe(fds, read_flags, write_flags));
    }

#endif
}

#endif //LIBUVCXX_PIPE_H
