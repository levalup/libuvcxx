//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_PIPE_H
#define LIBUVCXX_PIPE_H

#include "cxx/string.h"
#include "utils/promise2.h"

#include "connect.h"
#include "stream.h"

namespace uv {
    class pipe_t : public inherit_handle_t<uv_pipe_t, stream_t> {
    public:
        using self = pipe_t;
        using supper = inherit_handle_t<uv_pipe_t, stream_t>;

        using raw_t = uv_pipe_t;

        explicit pipe_t(bool ipc) : self(default_loop(), ipc) {}

        explicit pipe_t(const loop_t &loop, bool ipc) {
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_pipe_init(loop, *this, int(ipc));
            _initialized_();
        }

        self &detach() {
            _detach_();
            return *this;
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

        self accept(bool ipc) {
            self client(this->loop(), ipc);

            UVCXX_APPLY(uv_accept(*this, client), nullptr);

            client.data<data_t>()->work_mode = WorkMode::Agent;
            return client;
        }

        int open(uv_file file) {
            UVCXX_PROXY(uv_pipe_open(*this, file));
        }

        int bind(uvcxx::string name) {
            UVCXX_APPLY(uv_pipe_bind(*this, name), status);

            data<data_t>()->work_mode = WorkMode::Server;
            return 0;
        }

#if UVCXX_SATISFY_VERSION(1, 46, 0)

        int bind2(const char *name, size_t namelen, unsigned int flags) {
            UVCXX_APPLY(uv_pipe_bind2(*this, name, namelen, flags), status);

            data<data_t>()->work_mode = WorkMode::Server;
            return 0;
        }

        int bind2(uvcxx::string_view name, unsigned int flags) {
            return this->bind2(name.data, name.size, flags);
        }

#endif

        UVCXX_NODISCARD
        uvcxx::promise2<> connect(const connect_t &req, uvcxx::string name) {
            auto p = pipe_connect(req, *this, name);
            if (p) data<data_t>()->work_mode = WorkMode::Client;
            return {*this, p};
        }

        UVCXX_NODISCARD
        uvcxx::promise2<> connect(uvcxx::string name) {
            return this->connect({}, name);
        }

#if UVCXX_SATISFY_VERSION(1, 46, 0)

        UVCXX_NODISCARD
        uvcxx::promise2<> connect2(const connect_t &req, const char *name, size_t namelen, unsigned int flags) {
            auto p = pipe_connect2(req, *this, name, namelen, flags);
            if (p) data<data_t>()->work_mode = WorkMode::Client;
            return {*this, p};
        }

        UVCXX_NODISCARD
        uvcxx::promise2<> connect2(const char *name, size_t namelen, unsigned int flags) {
            return this->connect2({}, name, namelen, flags);
        }

        UVCXX_NODISCARD
        uvcxx::promise2<> connect2(const connect_t &req, uvcxx::string_view name, unsigned int flags) {
            return this->connect2(req, name.data, name.size, flags);
        }

        UVCXX_NODISCARD
        uvcxx::promise2<> connect2(uvcxx::string_view name, unsigned int flags) {
            return this->connect2({}, name.data, name.size, flags);
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
