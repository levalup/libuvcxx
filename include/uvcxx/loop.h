//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_LOOP_H
#define LIBUVCXX_LOOP_H

#include <atomic>
#include <cstring>
#include <memory>
#include <mutex>

#include <uv.h>

#include "cxx/except.h"
#include "cxx/version.h"
#include "cxx/wrapper.h"
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

        UVCXX_NODISCARD
        void *data() const {
            return raw()->data;
        }

#if UVCXX_SATISFY_VERSION(1, 0, 2)

        template<typename... Value>
        int configure(uv_loop_option option, Value ...value) {
            UVCXX_PROXY(uv_loop_configure(*this, option, value...));
        }

#endif

        int close() {
            auto data = (data_t *) raw()->data;
            if (data_t::is_it(data)) {
                UVCXX_PROXY(raw<loop_with_data_t>()->close());
            } else {
                UVCXX_PROXY(uv_loop_close(raw()));
            }
        }

        int run() {
            UVCXX_PROXY(uv_run(*this, UV_RUN_DEFAULT));
        }

        int run(uv_run_mode mode) {
            UVCXX_PROXY(uv_run(*this, mode));
        }

        UVCXX_NODISCARD
        bool alive() const {
            return uv_loop_alive(*this);
        }

        void stop() {
            uv_stop(*this);
        }

        UVCXX_NODISCARD
        static inline size_t size() {
            return uv_loop_size();
        }

        UVCXX_NODISCARD
        int backend_fd() const {
            return uv_backend_fd(*this);
        }

        UVCXX_NODISCARD
        int backend_timeout() const {
            return uv_backend_timeout(*this);
        }

        UVCXX_NODISCARD
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
            UVCXX_PROXY(uv_loop_fork(*this));
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
            UVCXX_PROXY(uv_metrics_info(*this, metrics));
        }

        uv_metrics_t metrics_info() {
            uv_metrics_t metrics{};
            std::memset(&metrics, 0, sizeof(uv_metrics_t));
            (void) metrics_info(&metrics);
            return metrics;
        }

#endif

        UVCXX_NODISCARD
        void *get_data() const {
            // cover uv_loop_get_data
            return raw()->data;
        }

        template<typename T>
        UVCXX_NODISCARD
        T *data() const {
            return (T *) raw()->data;
        }

        template<typename T>
        UVCXX_NODISCARD
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
        static inline self borrow(raw_t *raw) {
            return self{borrow_t(raw)};
        }

    private:
        class data_t {
        public:
            static constexpr uint64_t MAGIC = 0x1155665044332210;
            uint64_t magic = MAGIC;

            static inline bool is_it(void *data) {
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
                UVCXX_APPLY_STRICT(uv_loop_init(this), "can not init loop");
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
