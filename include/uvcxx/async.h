//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_ASYNC_H
#define LIBUVCXX_ASYNC_H

#include <uv.h>

#include "loop.h"
#include "handle.h"

#include "cxx/callback.h"

namespace uv {
    class async_t : public handle_extend_t<uv_async_t, handle_t> {
    public:
        using self = async_t;
        using supper = handle_extend_t<uv_async_t, handle_t>;

        class data_t : supper::data_t {
        public:
            loop_t loop;
            std::shared_ptr<async_t> ref;

            uvcxx::callback_emitter<> send_cb;

            // Store the instance of `handle` in `send_cb's wrapper`
            //     to avoid resource release caused by no external reference
            explicit data_t(loop_t loop, async_t async)
                : loop(std::move(loop)), ref(std::make_shared<async_t>(std::move(async))) {
            }

            void close() noexcept final {
                // finally at close, make queue safe
                send_cb.finally();
            }
        };

        async_t() : self(default_loop()) {}

        explicit async_t(loop_t loop) {
            // data will be deleted in close action
            set_data(new data_t(std::move(loop), *this));
        }

        uvcxx::callback<> init() {
            auto data = (data_t *)(get_data());

            auto err = uv_async_init(data->loop, *this, raw_callback);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);;
            return data->send_cb.callback();
        }

        int send() {
            auto err = uv_async_send(*this);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

    private:
        static void raw_callback(raw_t *handle) {
            auto data = (data_t *)(handle->data);
            data->send_cb.emit();
        }
    };
}

#endif //LIBUVCXX_ASYNC_H
