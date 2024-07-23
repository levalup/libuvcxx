//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_STREAM_H
#define LIBUVCXX_STREAM_H

#include "cxx/buffer.h"
#include "connect.h"
#include "handle.h"
#include "shutdown.h"
#include "write.h"

namespace uv {
    class stream_t : public handle_t {
    public:
        using self = stream_t;
        using supper = handle_t;
        using raw_t = uv_stream_t;

        using supper::supper;

        UVCXX_NODISCARD
        size_t write_queue_size() const {
            return raw<raw_t>()->write_queue_size;
        }

        UVCXX_NODISCARD
        uvcxx::promise<> shutdown(const shutdown_t &req) {
            return ::uv::shutdown(req, *this);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> shutdown() {
            return ::uv::shutdown(*this);
        }

        /**
         * @return uvcxx::callback<>
         * @throws E_ADDRINUSE, E_BADF, E_NOTSOCK
         */
        UVCXX_NODISCARD
        uvcxx::callback<> listen_callback() {
            return get_data<data_t>()->listen_cb.callback();
        }

        /**
         * Start listening for incoming connections. backlog indicates the number of connections the kernel might queue, same as listen(2).
         * @param backlog the number of connections the kernel might queue
         * @return uvcxx::callback<>
         * @throws E_ADDRINUSE, E_BADF, E_NOTSOCK
         */
        UVCXX_NODISCARD
        uvcxx::attached_callback<> listen(int backlog) {
            auto data = get_data<data_t>();
            if (data->work_mode != WorkMode::Notset && data->work_mode != WorkMode::Server) {
                UVCXX_THROW_OR_RETURN(UV_EPERM, nullptr, "can not listen ", work_mode_string(), " stream");
            }

            UVCXX_APPLY(uv_listen(*this, backlog, raw_listen_callback), nullptr);

            data->work_mode = WorkMode::Server;
            return {*this, data->listen_cb.callback()};
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
            // memory alloc can not register multi-times callback
            // use call(nullptr) to avoid call alloc multi times.
            return get_data<data_t>()->alloc_cb.callback().call(nullptr);
        }

        /**
         * @return uvcxx::callback<ssize_t, const uv_buf_t *>
         * @throws E_EOF, E_AGAIN
         */
        UVCXX_NODISCARD
        uvcxx::callback<ssize_t, const uv_buf_t *> read_callback() {
            return get_data<data_t>()->read_cb.callback();
        }

        /**
         * Read data from an incoming stream.
         * @return uvcxx::callback<ssize_t, const uv_buf_t *>
         * @throws E_EOF, E_AGAIN
         */
        UVCXX_NODISCARD
        uvcxx::attached_callback<ssize_t, const uv_buf_t *> read_start() {
            auto data = get_data<data_t>();
            if (data->work_mode == WorkMode::Server) {
                UVCXX_THROW_OR_RETURN(UV_EPERM, nullptr, "can not read ", work_mode_string(), " stream");
            }

            UVCXX_APPLY(uv_read_start(*this, raw_alloc_callback, raw_read_callback), nullptr);

            return {*this, data->read_cb.callback()};
        }

        int read_stop() {
            auto data = get_data<data_t>();
            if (data->work_mode == WorkMode::Server) {
                UVCXX_THROW_OR_RETURN(UV_EPERM, nullptr, "can not stop ", work_mode_string(), " stream");
            }

            UVCXX_PROXY(uv_read_stop(*this));
        }

        UVCXX_NODISCARD
        uvcxx::promise<> write(const write_t &req, const uv_buf_t bufs[], unsigned int nbufs) {
            return ::uv::write(req, *this, bufs, nbufs);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> write(const write_t &req, uvcxx::buffer buf) {
            return this->write(req, &buf.buf, 1);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> write(const write_t &req, std::initializer_list<uvcxx::buffer> bufs) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return this->write(req, buffers.data(), (unsigned int) buffers.size());
        }

        UVCXX_NODISCARD
        uvcxx::promise<> write(const uv_buf_t bufs[], unsigned int nbufs) {
            return ::uv::write(*this, bufs, nbufs);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> write(uvcxx::buffer buf) {
            return this->write(&buf.buf, 1);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> write(std::initializer_list<uvcxx::buffer> bufs) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return this->write(buffers.data(), (unsigned int) buffers.size());
        }

        UVCXX_NODISCARD
        uvcxx::promise<> write2(
                const write_t &req, const uv_buf_t bufs[], unsigned int nbufs, const stream_t &send_handle) {
            return ::uv::write2(req, *this, bufs, nbufs, send_handle);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> write2(const write_t &req,
                                uvcxx::mutable_buffer buf, const stream_t &send_handle) {
            return this->write2(req, &buf.buf, 1, send_handle);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> write2(const write_t &req,
                                std::initializer_list<uvcxx::mutable_buffer> bufs, const stream_t &send_handle) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return this->write2(req, buffers.data(), (unsigned int) buffers.size(), send_handle);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> write2(const uv_buf_t bufs[], unsigned int nbufs, const stream_t &send_handle) {
            return ::uv::write2(*this, bufs, nbufs, send_handle);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> write2(uvcxx::mutable_buffer buf, const stream_t &send_handle) {
            return this->write2(&buf.buf, 1, send_handle);
        }

        UVCXX_NODISCARD
        uvcxx::promise<> write2(std::initializer_list<uvcxx::mutable_buffer> bufs, const stream_t &send_handle) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return this->write2(buffers.data(), (unsigned int) buffers.size(), send_handle);
        }

        int try_write(const uv_buf_t bufs[], unsigned int nbufs) {
            return uv_try_write(*this, bufs, nbufs);
        }

        int try_write(uvcxx::buffer buf) {
            return try_write(&buf.buf, 1);
        }

        int try_write(std::initializer_list<uvcxx::buffer> bufs) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return try_write(buffers.data(), (unsigned int) buffers.size());
        }

#if UVCXX_SATISFY_VERSION(1, 42, 0)

        int try_write2(const uv_buf_t bufs[], unsigned int nbufs, const stream_t &send_handle) {
            return uv_try_write2(*this, bufs, nbufs, send_handle);
        }

        int try_write2(uvcxx::mutable_buffer buf, const stream_t &send_handle) {
            return try_write2(&buf.buf, 1, send_handle);
        }

        int try_write2(std::initializer_list<uvcxx::mutable_buffer> bufs, const stream_t &send_handle) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return try_write2(buffers.data(), (unsigned int) buffers.size(), send_handle);
        }

#endif

        UVCXX_NODISCARD
        bool is_readable() const {
            return uv_is_readable(*this);
        }

        UVCXX_NODISCARD
        bool is_writable() const {
            return uv_is_writable(*this);
        }

        int set_blocking(int blocking) {
            UVCXX_PROXY(uv_stream_set_blocking(*this, blocking));
        }

#if UVCXX_SATISFY_VERSION(1, 19, 0)

        UVCXX_NODISCARD
        size_t get_write_queue_size() const {
            return uv_stream_get_write_queue_size(*this);
        }

#endif

    public:
        operator uv_stream_t *() { return raw<raw_t>(); }

        operator uv_stream_t *() const { return raw<raw_t>(); }

        static inline self borrow(raw_t *raw) {
            return self{borrow_t(raw)};
        }

    private:
        static void raw_alloc_callback(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
            auto data = (data_t * )(handle->data);
            data->alloc_cb.emit(suggested_size, buf);
        }

        static void raw_read_callback(raw_t *handle, ssize_t nread, const uv_buf_t *buf) {
            auto data = (data_t * )(handle->data);
            if (nread > 0) {
                data->read_cb.emit(nread, buf);
                return;
            }
            switch (nread) {
                case 0:
                    break;
                case UV_EAGAIN:
                    data->read_cb.raise<uvcxx::E_AGAIN>();
                    break;
                case UV_EOF:
                    data->read_cb.raise<uvcxx::E_EOF>();
                    break;
                default:
                    data->read_cb.raise<uvcxx::errcode>(nread);
                    break;
            }
        }

        static void raw_listen_callback(raw_t *handle, int status) {
            auto data = (data_t * )(handle->data);
            if (status >= 0) {
                data->listen_cb.emit();
                return;
            }
            switch (status) {
                case UV_EADDRINUSE:
                    data->listen_cb.raise<uvcxx::E_ADDRINUSE>();
                    break;
                case UV_EBADF:
                    data->listen_cb.raise<uvcxx::E_BADF>();
                    break;
                case UV_ENOTSOCK:
                    data->listen_cb.raise<uvcxx::E_NOTSOCK>();
                    break;
                default:
                    data->listen_cb.raise<uvcxx::errcode>(status);
                    break;
            }
        }

    protected:
        enum class WorkMode {
            Notset = 0,
            Server = 1,
            Client = 2,
            Agent = 3,
        };

        UVCXX_NODISCARD
        const char *work_mode_string() const {
            auto data = get_data<data_t>();
            switch (data->work_mode) {
                case WorkMode::Notset:
                    return "notset";
                case WorkMode::Server:
                    return "server";
                case WorkMode::Client:
                    return "client";
                case WorkMode::Agent:
                    return "agent";
            }
        }

        class data_t : supper::data_t {
        public:
            uvcxx::callback_emitter<> listen_cb;
            uvcxx::callback_emitter<ssize_t, const uv_buf_t *> read_cb;
            uvcxx::callback_emitter<size_t, uv_buf_t *> alloc_cb;

            WorkMode work_mode{WorkMode::Notset};

            explicit data_t(stream_t &handle)
                    : supper::data_t(handle) {
                handle.watch(listen_cb);
                handle.watch(read_cb);
                handle.watch(alloc_cb);
            }

            ~data_t() override {
                alloc_cb.finalize();
                read_cb.finalize();
                listen_cb.finalize();
            }
        };
    };

    UVCXX_NODISCARD
    inline stream_t connect_t::handle() const {
        return stream_t::borrow(raw<raw_t>()->handle);
    }

    UVCXX_NODISCARD
    inline stream_t shutdown_t::handle() const {
        return stream_t::borrow(raw<raw_t>()->handle);
    }

    UVCXX_NODISCARD
    inline stream_t write_t::handle() const {
        return stream_t::borrow(raw<raw_t>()->handle);
    }

    UVCXX_NODISCARD
    inline stream_t write_t::send_handle() const {
        return stream_t::borrow(raw<raw_t>()->send_handle);
    }
}

#endif //LIBUVCXX_STREAM_H
