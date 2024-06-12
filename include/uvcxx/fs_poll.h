//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_FS_POLL_H
#define LIBUVCXX_FS_POLL_H

#include "handle.h"
#include "platform.h"

namespace uv {
    class fs_poll_t : public handle_extend_t<uv_fs_poll_t, handle_t> {
    public:
        using self = fs_poll_t;
        using supper = handle_extend_t<uv_fs_poll_t, handle_t>;

        class data_t : supper::data_t {
        public:
            using self = data_t;
            using supper = supper::data_t;

            uvcxx::callback_emitter<int, const uv_stat_t *, const uv_stat_t *> start_cb;

            explicit data_t(fs_poll_t &handle)
                    : supper(handle) {
                handle.watch(start_cb);
            }

            void close() noexcept final {
                // finally at close, make queue safe
                start_cb.finalize();
            }
        };

        fs_poll_t() : self(default_loop()) {}

        explicit fs_poll_t(const loop_t &loop) {
            (void) uv_fs_poll_init(loop, *this);
            // data will be deleted in close action
            set_data(new data_t(*this));
        }

        [[nodiscard]]
        uvcxx::callback<int, const uv_stat_t *, const uv_stat_t *> start(const char *path, unsigned int interval) {
            auto err = uv_fs_poll_start(*this, raw_callback, path, interval);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            auto data = get_data<data_t>();
            return data->start_cb.callback();
        }

        void stop() {
            (void) uv_fs_poll_stop(*this);
        }

        int getpath(char *buffer, size_t *size) {
            return uv_fs_poll_getpath(*this, buffer, size);
        }

    private:
        static void raw_callback(raw_t *handle, int status, const uv_stat_t *prev, const uv_stat_t *curr) {
            auto data = (data_t *) (handle->data);
            data->start_cb.emit(status, prev, curr);
        }
    };
}

#endif //LIBUVCXX_FS_POLL_H
