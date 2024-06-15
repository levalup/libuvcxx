//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_PREPARE_H
#define LIBUVCXX_PREPARE_H

#include "handle.h"

namespace uv {
    class prepare_t : public inherit_handle_t<uv_prepare_t, handle_t> {
    public:
        using self = prepare_t;
        using supper = inherit_handle_t<uv_prepare_t, handle_t>;

        prepare_t() : self(default_loop()) {}

        explicit prepare_t(const loop_t &loop) {
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_prepare_init(loop, *this);
        }

        [[nodiscard]]
        uvcxx::callback<> start() {
            auto err = uv_prepare_start(*this, raw_callback);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            auto data = get_data<data_t>();
            return data->start_cb.callback();
        }

        void stop() {
            (void) uv_prepare_stop(*this);
        }

    private:
        static void raw_callback(raw_t *handle) {
            auto data = (data_t *) (handle->data);
            data->start_cb.emit();
        }

        class data_t : supper::data_t {
        public:
            using self = data_t;
            using supper = supper::data_t;

            uvcxx::callback_emitter<> start_cb;

            explicit data_t(prepare_t &handle)
                    : supper(handle) {
                handle.watch(start_cb);
            }

            void close() noexcept final {
                start_cb.finalize();
            }
        };
    };
}

#endif //LIBUVCXX_PREPARE_H
