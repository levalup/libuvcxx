//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_FS_EVENT_H
#define LIBUVCXX_FS_EVENT_H

#include "handle.h"
#include "utils/platform.h"

namespace uv {
    class fs_event_t : public inherit_handle_t<uv_fs_event_t, handle_t> {
    public:
        using self = fs_event_t;
        using supper = inherit_handle_t<uv_fs_event_t, handle_t>;

        fs_event_t() : self(default_loop()) {}

        explicit fs_event_t(const loop_t &loop) {
#if !(UVCXX_OS_MAC || UVCXX_OS_WINDOWS)
            throw uvcxx::errcode(UV_EPERM, "current os does not support `fs_event`");
#endif
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_fs_event_init(loop, *this);
            _attach_close_();
        }

        [[nodiscard]]
        uvcxx::callback<const char *, uv_fs_event, int> start(const char *path, int flags) {
            auto err = uv_fs_event_start(*this, raw_callback, path, flags);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            _detach_();
            return get_data<data_t>()->start_cb.callback();
        }

        void stop() {
            (void) uv_fs_event_stop(*this);
            _attach_close_();
        }

        int getpath(char *buffer, size_t *size) {
            return uv_fs_event_getpath(*this, buffer, size);
        }

    private:
        static void raw_callback(raw_t *handle, const char *filename, int events, int status) {
            auto data = (data_t *) (handle->data);
            data->start_cb.emit(filename, uv_fs_event(events), status);
        }

        class data_t : supper::data_t {
        public:
            uvcxx::callback_emitter<const char *, uv_fs_event, int> start_cb;

            explicit data_t(fs_event_t &handle)
                    : supper::data_t(handle) {
                handle.watch(start_cb);
            }

            ~data_t() override {
                start_cb.finalize();
            }
        };
    };
}

#endif //LIBUVCXX_FS_EVENT_H
