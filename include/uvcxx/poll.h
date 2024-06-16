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

        self &init(const loop_t &loop, int fd) {
            auto data = get_data<data_t>();
            auto err = uv_poll_init(loop, *this, fd);
            // To directly start after init, there is no path to return the error code instead.
            // So an exception is directly thrown. This feature may be modified in the future.
            if (err < 0) throw uvcxx::errcode(err);
            data->initialized = true;
            return *this;
        }

        self &init_socket(const loop_t &loop, uv_os_sock_t socket) {
            auto data = get_data<data_t>();
            auto err = uv_poll_init_socket(loop, *this, socket);
            // To directly start after init, there is no path to return the error code instead.
            // So an exception is directly thrown. This feature may be modified in the future.
            if (err < 0) throw uvcxx::errcode(err);
            data->initialized = true;
            return *this;
        }

        self &init(int fd) {
            return init(default_loop(), fd);
        }

        self &init_socket(uv_os_sock_t socket) {
            return init_socket(default_loop(), socket);
        }

        [[nodiscard]]
        uvcxx::callback<int, int> start(int events) {
            auto data = get_data<data_t>();

            if (!data->initialized) {
                UVCXX_THROW_OR_RETURN(UV_EINVAL, nullptr, "should call `init` or `init_socket` first");
            }

            auto err = uv_poll_start(*this, events, raw_callback);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            return data->start_cb.callback();
        }

        void stop() {
            (void) uv_poll_stop(*this);
        }

    private:
        static void raw_callback(raw_t *handle, int status, int events) {
            auto data = (data_t *) (handle->data);
            data->start_cb.emit(status, events);
        }

        class data_t : supper::data_t {
        public:
            bool initialized{false};
            uvcxx::callback_emitter<int, int> start_cb;

            explicit data_t(poll_t &handle)
                    : supper::data_t(handle) {
                handle.watch(start_cb);
            }

            void close() noexcept final {
                start_cb.finalize();
            }
        };
    };
}

#endif //LIBUVCXX_POLL_H
