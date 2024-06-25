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

#include "cxx/except.h"
#include "cxx/version.h"
#include "inner/base.h"
#include "utils/defer.h"

namespace uv {
    /**
     * The data field of uv_loop_t will be retained and used by the UVCXX.
     * Please do not modify data.
     */
    class loop_t : public uvcxx::shared_raw_base_t<uv_loop_t> {
    public:
        using self = loop_t;
        using supper = uvcxx::shared_raw_base_t<uv_loop_t>;

        using supper::supper;

        loop_t() : self(std::make_shared<loop_with_data_t>()) {}

        [[nodiscard]]
        void *data() const {
            return raw()->data;
        }

#if UVCXX_SATISFY_VERSION(1, 0, 2)

        template<typename... Value>
        int configure(uv_loop_option option, Value ...value) {
            auto err = uv_loop_configure(*this, option, value...);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

#endif

        int close() {
            auto data = (data_t *) raw()->data;
            if (data_t::is_it(data)) {
                auto status = raw<loop_with_data_t>()->close();
                if (status < 0) UVCXX_THROW_OR_RETURN(status, status);
                return status;
            } else {
                auto status = uv_loop_close(raw());
                if (status < 0) UVCXX_THROW_OR_RETURN(status, status);
                return status;
            }
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

        void stop() {
            uv_stop(*this);
        }

        [[nodiscard]]
        static size_t size() {
            return uv_loop_size();
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

        void walk(uv_walk_cb walk_cb, void *arg = nullptr) {
            uv_walk(*this, walk_cb, arg);
        }

#if UVCXX_SATISFY_VERSION(1, 12, 0)

        int fork() {
            auto err = uv_loop_fork(*this);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

#endif

#if UVCXX_SATISFY_VERSION(1, 8, 0)

        void print_all_handles(FILE *stream) {
            uv_print_all_handles(*this, stream);
        }

        void print_active_handles(FILE *stream) {
            uv_print_active_handles(*this, stream);
        }

#endif

#if UVCXX_SATISFY_VERSION(1, 39, 0)

        uint64_t metrics_idle_time() {
            return uv_metrics_idle_time(*this);
        }

#endif

#if UVCXX_SATISFY_VERSION(1, 45, 0)

        int metrics_info(uv_metrics_t *metrics) {
            return uv_metrics_info(*this, metrics);
        }

        uv_metrics_t metrics_info() {
            uv_metrics_t metrics{};
            (void) uv_metrics_info(*this, &metrics);
            return metrics;
        }

#endif

        [[nodiscard]]
        void *get_data() const {
            // cover uv_loop_get_data
            return raw()->data;
        }

        template<typename T>
        [[nodiscard]]
        T *data() const {
            return (T *) raw()->data;
        }

        template<typename T>
        [[nodiscard]]
        T *get_data() const {
            return (T *) raw()->data;
        }

        /**
         * DO NOT this method on your own as it may result in unexpected failure.
         * @param data
         */
        void set_data(void *data) {
            // cover uv_loop_set_data
            raw()->data = data;
        }

    public:
        static self borrow(raw_t *raw) {
            return self{borrow_t(raw)};
        }

    private:
        class data_t {
        public:
            static constexpr uint64_t MAGIC = 0x1155665044332210;
            uint64_t magic = MAGIC;

            static bool is_it(void *data) {
                return data && ((data_t *) data)->magic == MAGIC;
            }

            virtual ~data_t() = default;

        public:
            bool close_for(const std::function<void()> &close) {
                bool closed = false;
                if (!m_closed.compare_exchange_strong(closed, true)) return false;

                close();

                return true;
            }

        private:
            std::atomic<bool> m_closed{false};
        };

        class loop_with_data_t : public uv_loop_t {
        public:
            loop_with_data_t() : uv_loop_t() {
                auto status = uv_loop_init(this);
                if (status < 0) throw uvcxx::errcode(status, "can not init loop");
                this->data = &m_data;
            }

            loop_with_data_t(const loop_with_data_t &) = delete;

            loop_with_data_t &operator=(const loop_with_data_t &) = delete;

            ~loop_with_data_t() {
                (void) m_data.close_for([this]() {
                    (void) uv_loop_close(this);
                });
            }

            int close() {
                int status = 0;
                (void) m_data.close_for([&]() {
                    status = uv_loop_close(this);
                });
                return status;
            }

        private:
            data_t m_data;
        };
    };

    inline loop_t default_loop() {
        return loop_t::borrow(uv_default_loop());
    }
}

#endif // LIBUVCXX_LOOP_H
