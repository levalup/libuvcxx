//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_POLL_H
#define LIBUVCXX_POLL_H

#include <uv.h>

#include "loop.h"
#include "handle.h"

#include "cxx/callback.h"

namespace uv {
    class poll_t : public handle_extend_t<uv_poll_t, handle_t> {
    public:
        using self = poll_t;
        using supper = handle_extend_t<uv_poll_t, handle_t>;

        class data_t : supper::data_t {
        public:
            using self = data_t;
            using supper = supper::data_t;

            loop_t loop;
            uvcxx::callback_emitter<int, int> start_cb;

            explicit data_t(poll_t &handle, loop_t loop)
                    : supper(handle), loop(std::move(loop)) {
                handle.watch(start_cb);
            }

            void close() noexcept final {
                // finally at close, make queue safe
                start_cb.finalize();
            }
        };

        poll_t() : self(default_loop()) {}

        explicit poll_t(loop_t loop) {
            // data will be deleted in close action
            set_data(new data_t(*this, std::move(loop)));
        }

        self &init(int fd) {
            auto data = get_data<data_t>();
            uv_poll_init(data->loop, *this, fd);
            return *this;
        }

        self &init_socket(uv_os_sock_t socket) {
            auto data = get_data<data_t>();
            uv_poll_init_socket(data->loop, *this, socket);
            return *this;
        }

        [[nodiscard]]
        uvcxx::callback<int, int> start(int events) {
            auto err = uv_poll_start(*this, events, raw_callback);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            auto data = (data_t *) (get_data());
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
