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

        class data_t : supper::data_t {
        public:
            using self = data_t;
            using supper = supper::data_t;

            uvcxx::callback_emitter<int> listen_cb;
            uvcxx::callback_emitter<ssize_t, const uv_buf_t *> read_cb;
            uvcxx::callback_emitter<size_t, uv_buf_t *> alloc_cb;

            explicit data_t(stream_t &handle)
                    : supper(handle) {
                handle.watch(listen_cb);
                handle.watch(read_cb);
                handle.watch(alloc_cb);
            }

            void close() noexcept override {
                alloc_cb.finalize();
                read_cb.finalize();
                listen_cb.finalize();
                supper::close();
            }
        };

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
            auto err = uv_listen(*this, backlog, raw_listen_callback);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

            auto data = this->data<data_t>();
            return data->listen_cb.callback();
        }

        int accept(stream_t &client) {
            auto err = uv_accept(*this, client);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        [[nodiscard]]
        uvcxx::callback<size_t, uv_buf_t *> alloc() {
            auto data = get_data<data_t>();
            // memory alloc can not register multi-times callback
            return data->alloc_cb.callback().call(nullptr);
        }

        [[nodiscard]]
        uvcxx::callback<ssize_t, const uv_buf_t *> read_start() {
            auto err = uv_read_start(*this, raw_alloc_callback, raw_read_callback);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            auto data = get_data<data_t>();
            return data->read_cb.callback();
        }

        void read_stop() {
            uv_read_stop(*this);
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

        int try_write2(const uv_buf_t bufs[], unsigned int nbufs, uv_stream_t *send_handle) noexcept {
            return uv_try_write2(*this, bufs, nbufs, send_handle);
        }

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

        [[nodiscard]]
        size_t get_write_queue_size() const {
            return uv_stream_get_write_queue_size(*this);
        }

        operator raw_t *() { return raw<raw_t>(); }

        operator raw_t *() const { return raw<raw_t>(); }

        static self borrow(raw_t *raw) {
            return self{borrow_t(raw)};
        }

    private:
        static void raw_alloc_callback(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
            auto data = (data_t *) (handle->data);
            data->alloc_cb.emit(suggested_size, buf);
        }

        static void raw_read_callback(raw_t *handle, ssize_t nread, const uv_buf_t *buf) {
            auto data = (data_t *) (handle->data);
            data->read_cb.emit(nread, buf);
        }

        static void raw_listen_callback(raw_t *handle, int status) {
            auto data = (data_t *) (handle->data);
            data->listen_cb.emit(status);
        }
    };

    class acceptable_stream_t : public stream_t {
    public:
        using self = stream_t;
        using supper = stream_t;

        using supper::supper;

        class data_t : supper::data_t {
        public:
            using self = data_t;
            using supper = stream_t::data_t;

            uvcxx::callback_emitter<stream_t> accept_cb;

            explicit data_t(acceptable_stream_t &handle)
                    : supper(handle) {
                handle.watch(accept_cb);
            }

            void close() noexcept override {
                accept_cb.finalize();
                supper::close();
            }
        };

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
