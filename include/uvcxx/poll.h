//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_POLL_H
#define LIBUVCXX_POLL_H

#include "handle.h"

namespace uv {
    class poll_t : public handle_extend_t<uv_poll_t, handle_t> {
    public:
        using self = poll_t;
        using supper = handle_extend_t<uv_poll_t, handle_t>;

        class data_t : supper::data_t {
        public:
            using self = data_t;
            using supper = supper::data_t;

            bool initialized{false};
            uvcxx::callback_emitter<int, int> start_cb;

            explicit data_t(poll_t &handle)
                    : supper(handle) {
                handle.watch(start_cb);
            }

            void close() noexcept final {
                // finally at close, make queue safe
                start_cb.finalize();
            }
        };

        poll_t() {
            // data will be deleted in close action
            set_data(new data_t(*this));
        }

        self &init(const loop_t &loop, int fd) {
            auto data = get_data<data_t>();
            auto err = uv_poll_init(loop, *this, fd);
            if (err < 0) throw uvcxx::exception(err);
            data->initialized = true;
            return *this;
        }

        self &init_socket(const loop_t &loop, uv_os_sock_t socket) {
            auto data = get_data<data_t>();
            auto err = uv_poll_init_socket(loop, *this, socket);
            if (err < 0) throw uvcxx::exception(err);
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
            auto data = (data_t *) (get_data());

            if (!data->initialized) UVCXX_THROW_OR_RETURN(UV_EINVAL, nullptr);

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
    };
}

#endif //LIBUVCXX_POLL_H
