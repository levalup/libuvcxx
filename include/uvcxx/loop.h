//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_LOOP_H
#define LIBUVCXX_LOOP_H

#include <atomic>
#include <memory>
#include <mutex>
#include <cassert>

#include <uv.h>

#include "cxx/defer.h"
#include "cxx/except.h"
#include "inner/base.h"

namespace uv {
    class loop_t : public uvcxx::shared_raw_base_t<uv_loop_t> {
    public:
        using self = loop_t;
        using supper = uvcxx::shared_raw_base_t<uv_loop_t>;

        using supper::supper;

        class data_t {
        public:
            static constexpr uint64_t MAGIC = 0x1155665044332210;
            uint64_t magic = MAGIC;

            // use to make sure this loop close.
            std::mutex closing;
            std::atomic<bool> closed{false};

            static bool is_it(void *data) {
                return data && ((data_t *) data)->magic == MAGIC;
            }

        public:
            virtual ~data_t() = default;
        };

        loop_t() : self(make_shared()) {}

        int close() {
            auto data = (data_t *) raw()->data;
            if (data_t::is_it(data)) {
                auto err = close_loop(*this, data);
                if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
                return err;
            } else {
                auto err = uv_loop_close(raw());
                if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
                return err;
            }
        }

        template<typename T>
        int configure(uv_loop_option option, T value) {
            auto err = uv_loop_configure(*this, option, value);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int run() {
            auto err = uv_run(*this, UV_RUN_DEFAULT);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int run(uv_run_mode mode) {
            auto err = uv_run(*this, mode);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        [[nodiscard]]
        bool alive() const {
            return uv_loop_alive(*this);
        }

        [[nodiscard]]
        int backend_fd() const {
            return uv_backend_fd(*this);
        }

        [[nodiscard]]
        int backend_timeout() const {
            return uv_backend_timeout(*this);
        }

        [[nodiscard]]
        uint64_t now() const {
            return uv_now(*this);
        }

        void update_time() {
            uv_update_time(*this);
        }

        void walk(uv_walk_cb walk_cb, void *arg) {
            uv_walk(*this, walk_cb, arg);
        }

        int fork() {
            auto err = uv_loop_fork(*this);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        uint64_t metrics_idle_time() {
            return uv_metrics_idle_time(*this);
        }

        int metrics_info(uv_metrics_t *metrics) {
            return uv_metrics_info(*this, metrics);
        }

        uv_metrics_t metrics_info() {
            uv_metrics_t metrics{};
            (void) uv_metrics_info(*this, &metrics);
            return metrics;
        }

        [[nodiscard]]
        void *get_data() const {
            return uv_loop_get_data(*this);
        }

        /**
         * Never use this method on your own as it may result in failure.
         * @param data
         */
        void set_data(void *data) {
            uv_loop_set_data(*this, data);
        }

        static self borrow(raw_t *raw) {
            return self{borrow_t(raw)};
        }

    private:
        static int close_loop(raw_t *raw, data_t *data) {
            if (!data) return 0;

            std::unique_lock _lock(data->closing);
            if (data->closed) return 0;

            auto err = uv_loop_close(raw);
            if (err < 0) return err;

            data->closed.store(true);
            return 0;
        }

        static std::shared_ptr<raw_t> make_shared() {
            using namespace uvcxx;

            auto data = new data_t;
            defer delete_data(std::default_delete<data_t>(), data);

            auto raw = new raw_t;
            defer delete_raw(std::default_delete<raw_t>(), raw);
            raw->data = data;

            auto err = uv_loop_init(raw);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

            auto result = std::shared_ptr<raw_t>(raw, [data](raw_t *raw) {
                defer delete_raw(std::default_delete<raw_t>(), raw);
                defer delete_data(std::default_delete<data_t>(), data);

                auto err = close_loop(raw, data);
                if (err < 0) UVCXX_THROW(err);
            });

            delete_data.release();
            delete_raw.release();
            return result;
        }
    };

    inline loop_t default_loop() {
        return loop_t::borrow(uv_default_loop());
    }
}

#endif // LIBUVCXX_LOOP_H
