//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_POLL_H
#define LIBUVCXX_POLL_H

#include "handle.h"

namespace uv {
    class poll_t : public inherit_handle_t<uv_poll_t, handle_t> {
    public:
        using self = poll_t;
        using supper = inherit_handle_t<uv_poll_t, handle_t>;

        poll_t() {
            set_data(new data_t(*this));    //< data will be deleted in close action
        }

        self &detach() {
            _detach_();
            return *this;
        }

        self &init(const loop_t &loop, int fd) {
            auto data = get_data<data_t>();
            if (data->is_initialized()) {
                UVCXX_THROW_OR_RETURN(UV_EINVAL, nullptr, "duplicated poll_t initialization");
            }
            // To directly start after init, there is no path to return the error code instead.
            // So an exception is directly thrown. This feature may be modified in the future.
            UVCXX_APPLY_STRICT(uv_poll_init(loop, *this, fd));
            _initialized_();
            return *this;
        }

        self &init_socket(const loop_t &loop, uv_os_sock_t socket) {
            auto data = get_data<data_t>();
            if (data->is_initialized()) {
                UVCXX_THROW_OR_RETURN(UV_EINVAL, nullptr, "duplicated poll_t initialization");
            }
            // To directly start after init, there is no path to return the error code instead.
            // So an exception is directly thrown. This feature may be modified in the future.
            UVCXX_APPLY_STRICT(uv_poll_init_socket(loop, *this, socket));
            _initialized_();
            return *this;
        }

        self &init(int fd) {
            return init(default_loop(), fd);
        }

        self &init_socket(uv_os_sock_t socket) {
            return init_socket(default_loop(), socket);
        }

        UVCXX_NODISCARD
        uvcxx::callback<int> callback() {
            return get_data<data_t>()->start_cb.callback();
        }

        UVCXX_NODISCARD
        uvcxx::callback2<int> start(int events) {
            auto data = get_data<data_t>();

            if (!data->is_initialized()) {
                UVCXX_THROW_OR_RETURN(UV_EINVAL, nullptr, "should call `init` or `init_socket` first");
            }

            UVCXX_APPLY(uv_poll_start(*this, events, raw_callback), nullptr);

            _detach_();
            return {*this, data->start_cb.callback()};
        }

        void stop() {
            auto data = get_data<data_t>();

            if (!data->is_initialized()) {
                UVCXX_THROW_OR_RETURN(UV_EINVAL, nullptr, "should call `init` or `init_socket` first");
            }

            (void) uv_poll_stop(*this);
        }

    private:
        static void raw_callback(raw_t *handle, int status, int events) {
            auto data = (data_t *) (handle->data);
            if (status >= 0) {
                data->start_cb.emit(events);
                return;
            }
            switch (status) {
                case UV_EAGAIN:
                    data->start_cb.raise<uvcxx::E_AGAIN>();
                    break;
                default:
                    data->start_cb.raise<uvcxx::errcode>(status);
                    break;
            }
        }

        class data_t : public supper::data_t {
        public:
            uvcxx::callback_emitter<int> start_cb;

            explicit data_t(poll_t &handle)
                    : supper::data_t(handle) {
                handle.watch(start_cb);
            }

            ~data_t() override {
                start_cb.finalize();
            }
        };
    };
}

#endif //LIBUVCXX_POLL_H
