//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_TTY_H
#define LIBUVCXX_TTY_H

#include "stream.h"

namespace uv {
    class tty_t : public inherit_handle_t<uv_tty_t, stream_t> {
    public:
        using self = tty_t;
        using supper = inherit_handle_t<uv_tty_t, stream_t>;

        tty_t(uv_file fd, int unused) : self(default_loop(), fd, unused) {}

        explicit tty_t(const loop_t &loop, uv_file fd, int unused) {
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_tty_init(loop, *this, fd, unused);
            _attach_close_();
        }

        int fileno(uv_os_fd_t *fd) const {
            UVCXX_PROXY(uv_fileno(*this, fd));
        }

        stream_t accept(uv_file fd, int unused) {
            self client(this->loop(), fd, unused);

            UVCXX_APPLY(uv_accept(*this, client), nullptr);

            return client;
        }

#if !UVCXX_SATISFY_VERSION(1, 2, 0)
        using uv_tty_mode_t = int;
#endif

        int set_mode(uv_tty_mode_t mode) {
            UVCXX_PROXY(uv_tty_set_mode(*this, mode));
        }

        int get_winsize(int *width, int *height) const {
            UVCXX_PROXY(uv_tty_get_winsize(*this, width, height));
        }
    };

    namespace tty {
        inline int reset_mode() {
            UVCXX_PROXY(uv_tty_reset_mode());
        }

#if UVCXX_SATISFY_VERSION(1, 33, 0)

        inline void set_vterm_state(uv_tty_vtermstate_t state) {
            uv_tty_set_vterm_state(state);
        }

#endif

#if UVCXX_SATISFY_VERSION(1, 40, 0)

        inline int get_vterm_state(uv_tty_vtermstate_t *state) {
            UVCXX_PROXY(uv_tty_get_vterm_state(state));
        }

        inline uv_tty_vtermstate_t get_vterm_state() {
            uv_tty_vtermstate_t state = UV_TTY_UNSUPPORTED;
            (void) get_vterm_state(&state);
            return state;
        }

#endif
    }
}

#endif //LIBUVCXX_TTY_H
