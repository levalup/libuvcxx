//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_LOOP_H
#define LIBUVCXX_LOOP_H

#include <memory>

#include <uv.h>

#include "cxx/defer.h"
#include "cxx/memory.h"
#include "cxx/except.h"

namespace uv {
    enum run_mode {
        RUN_DEFAULT = UV_RUN_DEFAULT,
        RUN_ONCE = UV_RUN_ONCE,
        RUN_NOWAIT = UV_RUN_NOWAIT,
    };

    class loop_t {
    public:
        using self = loop_t;
        using raw_t = uv_loop_t;

        loop_t()
            : self(uvcxx::make_shared<raw_t>(uv_loop_init, uv_loop_close)) {}

        static self init() { return self{ }; }

        operator raw_t *() { return m_raw.get(); }

        operator raw_t *() const { return m_raw.get(); }

        explicit operator bool() { return bool(m_raw); }

        int run() {
            auto err = uv_run(m_raw.get(), UV_RUN_DEFAULT);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int run(run_mode mode) {
            auto err = uv_run(m_raw.get(), uv_run_mode(mode));
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        static self borrow(uv_loop_t *loop) {
            return self{uvcxx::make_borrowed(loop)};
        }
    private:
        std::shared_ptr<raw_t> m_raw;

        explicit loop_t(decltype(m_raw) raw) : m_raw(std::move(raw)) {}

        friend loop_t default_loop();
        friend class handle_t;  // for handle_t::loop()
    };

    inline loop_t default_loop() {
        return loop_t::borrow(uv_default_loop());
    }
}

#endif // LIBUVCXX_LOOP_H
