//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_STREAM_H
#define LIBUVCXX_STREAM_H

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

        [[nodiscard]]
        size_t write_queue_size() const {
            return raw<raw_t>()->write_queue_size;
        }

        [[nodiscard]]
        uvcxx::promise<> shutdown(const shutdown_t &req) {
            return ::uv::shutdown(req, *this);
        }

        [[nodiscard]]
        uvcxx::promise<> shutdown() {
            return ::uv::shutdown(*this);
        }

        [[nodiscard]]
        uvcxx::callback<int> listen(int backlog) {
            auto data = get_data<data_t>();
            if (data->work_mode == WorkMode::Read) {
                UVCXX_THROW_OR_RETURN(UV_EPERM, nullptr, "can not listen reading stream");
            }
            data->work_mode = WorkMode::Listen;

            auto err = uv_listen(*this, backlog, raw_listen_callback);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

            _detach_();
            return data->listen_cb.callback();
        }

        int accept(stream_t &client) {
            auto err = uv_accept(*this, client);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        [[nodiscard]]
        uvcxx::callback<size_t, uv_buf_t *> alloc() {
            // this alloc is not under Running state, so no `_detach_` applied.
            // memory alloc can not register multi-times callback
            return get_data<data_t>()->alloc_cb.callback().call(nullptr);
        }

        [[nodiscard]]
        uvcxx::callback<ssize_t, const uv_buf_t *> read_start() {
            auto data = get_data<data_t>();
            if (data->work_mode == WorkMode::Listen) {
                UVCXX_THROW_OR_RETURN(UV_EPERM, nullptr, "can not read listening stream");
            }
            data->work_mode = WorkMode::Read;

            auto err = uv_read_start(*this, raw_alloc_callback, raw_read_callback);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

            _detach_();
            return data->read_cb.callback();
        }

        int read_stop() {
            auto data = get_data<data_t>();
            if (data->work_mode == WorkMode::Listen) {
                UVCXX_THROW_OR_RETURN(UV_EPERM, nullptr, "can not stop listening stream");
            }

            auto status = uv_read_stop(*this);
            if (status < 0) UVCXX_THROW_OR_RETURN(status, status);

            data->work_mode = WorkMode::Notset;

            _attach_close_();
            return status;
        }

        [[nodiscard]]
        uvcxx::promise<> write(const write_t &req, const uv_buf_t bufs[], unsigned int nbufs) {
            return ::uv::write(req, *this, bufs, nbufs);
        }

        [[nodiscard]]
        uvcxx::promise<> write(const uv_buf_t bufs[], unsigned int nbufs) {
            return ::uv::write(*this, bufs, nbufs);
        }

        [[nodiscard]]
        uvcxx::promise<> write2(
                const write_t &req, const uv_buf_t bufs[], unsigned int nbufs, uv_stream_t *send_handle) {
            return ::uv::write2(req, *this, bufs, nbufs, send_handle);
        }

        [[nodiscard]]
        uvcxx::promise<> write2(const uv_buf_t bufs[], unsigned int nbufs, uv_stream_t *send_handle) {
            return ::uv::write2(*this, bufs, nbufs, send_handle);
        }

        [[nodiscard]]
        uvcxx::promise<> write(
                const write_t &req, const uv_buf_t bufs[], unsigned int nbufs, uv_stream_t *send_handle) {
            return ::uv::write2(req, *this, bufs, nbufs, send_handle);
        }

        [[nodiscard]]
        uvcxx::promise<> write(const uv_buf_t bufs[], unsigned int nbufs, uv_stream_t *send_handle) {
            return ::uv::write2(*this, bufs, nbufs, send_handle);
        }

        int try_write(const uv_buf_t bufs[], unsigned int nbufs) noexcept {
            return uv_try_write(*this, bufs, nbufs);
        }

#if UVCXX_SATISFY_VERSION(1, 42, 0)

        int try_write2(const uv_buf_t bufs[], unsigned int nbufs, uv_stream_t *send_handle) noexcept {
            return uv_try_write2(*this, bufs, nbufs, send_handle);
        }

#endif

        [[nodiscard]]
        bool is_readable() const {
            return uv_is_readable(*this);
        }

        [[nodiscard]]
        bool is_writable() const {
            return uv_is_readable(*this);
        }

        int set_blocking(int blocking) {
            return uv_stream_set_blocking(*this, blocking);
        }

#if UVCXX_SATISFY_VERSION(1, 19, 0)

        [[nodiscard]]
        size_t get_write_queue_size() const {
            return uv_stream_get_write_queue_size(*this);
        }

#endif

    public:
        operator uv_stream_t *() { return raw<raw_t>(); }

        operator uv_stream_t *() const { return raw<raw_t>(); }

        static self borrow(raw_t *raw) {
            return self{borrow_t(raw)};
        }

    private:
        static void raw_alloc_callback(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
            auto data = (data_t * )(handle->data);
            data->alloc_cb.emit(suggested_size, buf);
        }

        static void raw_read_callback(raw_t *handle, ssize_t nread, const uv_buf_t *buf) {
            auto data = (data_t * )(handle->data);
            data->read_cb.emit(nread, buf);
        }

        static void raw_listen_callback(raw_t *handle, int status) {
            auto data = (data_t * )(handle->data);
            data->listen_cb.emit(status);
        }

    protected:
        enum class WorkMode {
            Notset = 0,
            Listen = 1,
            Read = 2,
        };

        class data_t : supper::data_t {
        public:
            uvcxx::callback_emitter<int> listen_cb;
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

    class acceptable_stream_t : public stream_t {
    public:
        using self = stream_t;
        using supper = stream_t;

        using supper::supper;

        virtual stream_t accept() = 0;

        [[nodiscard]]
        uvcxx::callback<stream_t> listen_accept(int backlog) {
            this->listen(backlog).call(nullptr).call([this](int status) {
                auto data = this->data<data_t>();
                if (status < 0) {
                    data->accept_cb.raise<uvcxx::errcode>(status);
                } else {
                    data->accept_cb.emit(this->accept());
                }
            });
            auto data = this->data<data_t>();
            return data->accept_cb.callback();
        }

    protected:
        class data_t : supper::data_t {
        public:
            uvcxx::callback_emitter<stream_t> accept_cb;

            explicit data_t(acceptable_stream_t &handle)
                    : supper::data_t(handle) {
                handle.watch(accept_cb);
            }

            ~data_t() override {
                accept_cb.finalize();
            }
        };
    };

    [[nodiscard]]
    inline stream_t connect_t::handle() const {
        return stream_t::borrow(raw<raw_t>()->handle);
    }

    [[nodiscard]]
    inline stream_t shutdown_t::handle() const {
        return stream_t::borrow(raw<raw_t>()->handle);
    }

    [[nodiscard]]
    inline stream_t write_t::handle() const {
        return stream_t::borrow(raw<raw_t>()->handle);
    }

    [[nodiscard]]
    inline stream_t write_t::send_handle() const {
        return stream_t::borrow(raw<raw_t>()->send_handle);
    }
}

#endif //LIBUVCXX_STREAM_H
