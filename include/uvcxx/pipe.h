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

        int open(uv_file file) {
            auto err = uv_pipe_open(*this, file);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int bind(uvcxx::string name) {
            auto err = uv_pipe_bind(*this, name);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

#if UVCXX_SATISFY_VERSION(1, 46, 0)

        int bind2(const char *name, size_t namelen, unsigned int flags) {
            auto err = uv_pipe_bind2(*this, name, namelen, flags);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
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
            return ::uv::pipe_connect(req, *this, name);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect(uvcxx::string name) {
            return ::uv::pipe_connect(*this, name);
        }

#if UVCXX_SATISFY_VERSION(1, 46, 0)

        UVCXX_NODISCARD
        uvcxx::promise<> connect2(const connect_t &req, const char *name, size_t namelen, unsigned int flags) {
            return ::uv::pipe_connect2(req, *this, name, namelen, flags);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect2(const char *name, size_t namelen, unsigned int flags) {
            return ::uv::pipe_connect2(*this, name, namelen, flags);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect2(const connect_t &req, uvcxx::string_view name, unsigned int flags) {
            return ::uv::pipe_connect2(req, *this, name.data, name.size, flags);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect2(uvcxx::string_view name, unsigned int flags) {
            return ::uv::pipe_connect2(*this, name.data, name.size, flags);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect(const connect_t &req, uvcxx::string_view name, unsigned int flags) {
            return ::uv::pipe_connect2(req, *this, name.data, name.size, flags);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> connect(uvcxx::string_view name, unsigned int flags) {
            return ::uv::pipe_connect2(*this, name.data, name.size, flags);
        }

#endif

        int getsockname(char *buffer, size_t *size) const {
            auto err = uv_pipe_getsockname(*this, buffer, size);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

#if UVCXX_SATISFY_VERSION(1, 3, 0)

        int getpeername(char *buffer, size_t *size) const {
            auto err = uv_pipe_getpeername(*this, buffer, size);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
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
            auto err = uv_pipe_chmod(*this, flags);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

#endif
    };

#if UVCXX_SATISFY_VERSION(1, 41, 0)

    inline int pipe(uv_file fds[2], int read_flags, int write_flags) {
        return uv_pipe(fds, read_flags, write_flags);
    }

#endif
}

#endif //LIBUVCXX_PIPE_H
