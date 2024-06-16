//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_THREAD_H
#define LIBUVCXX_THREAD_H

#include <atomic>
#include <functional>

#include <uv.h>

#include "uvcxx/cxx/except.h"
#include "uvcxx/cxx/version.h"
#include "uvcxx/inner/base.h"

namespace uv {
    namespace inner {
        class thread_t {
        public:
            using self = thread_t;
            using raw_t = uv_thread_t;

            using entry_t = std::function<void(void)>;

            raw_t tid{};
            std::atomic<bool> m_joined{false};

            thread_t(const thread_t &) = delete;

            thread_t &operator=(const thread_t &) = delete;

            explicit thread_t(entry_t entry)
                    : m_entry(std::move(entry)) {
                auto err = uv_thread_create(&tid, raw_entry, this);
                if (err < 0) throw uvcxx::errcode(err);
            }

#if UVCXX_SATISFY_VERSION(1, 26, 0)

            explicit thread_t(entry_t entry, const uv_thread_options_t *params)
                    : m_entry(std::move(entry)) {
                auto err = uv_thread_create_ex(&tid, params, raw_entry, this);
                if (err < 0) throw uvcxx::errcode(err);
            }

#endif

            int join() {
                bool joined = false;
                if (!m_joined.compare_exchange_strong(joined, true)) { return 0; }

                auto err = uv_thread_join(&tid);
                if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
                return err;
            }


        private:
            entry_t m_entry;

            static void raw_entry(void *arg) {
                try {
                    ((self *) arg)->m_entry();
                } catch (...) {
                }
            }
        };
    }

    class thread_t final : public uvcxx::base_t {
    public:
        using self = thread_t;
        using raw_t = uv_thread_t;

        using inner_t = inner::thread_t;
        using entery_t = inner_t::entry_t;

        thread_t(const thread_t &) = delete;

        thread_t &operator=(const thread_t &) = delete;

        thread_t(thread_t &&that) noexcept = default;

        thread_t &operator=(thread_t &&that) noexcept = default;

        thread_t(std::nullptr_t) {}

        explicit thread_t(entery_t entry)
                : m_thread(new inner_t(std::move(entry))) {}

#if UVCXX_SATISFY_VERSION(1, 26, 0)

        explicit thread_t(entery_t entry, const uv_thread_options_t *params)
                : m_thread(new inner_t(std::move(entry), params)) {}

#endif

        ~thread_t() {
            join();
            delete m_thread;
        }

        int join() {
            if (m_thread) return m_thread->join();
            return 0;
        }

#if UVCXX_SATISFY_VERSION(1, 45, 0)

        int setaffinity(char *cpumask, char *oldmask, size_t mask_size) {
            return uv_thread_setaffinity(*this, cpumask, oldmask, mask_size);
        }

        int getaffinity(char *cpumask, size_t mask_size) const {
            return uv_thread_getaffinity(*this, cpumask, mask_size);
        }

#endif

        [[nodiscard]]
        bool equal(const self &other) const {
            return uv_thread_equal(*this, other);
        }

#if UVCXX_SATISFY_VERSION(1, 48, 0)

        int setpriority(int priority) {
            auto err = uv_thread_setpriority(m_thread->tid, priority);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int getpriority(int *priority) const {
            auto err = uv_thread_getpriority(m_thread->tid, priority);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        [[nodiscard]]
        int getpriority() const {
            int priority = 0;
            auto err = uv_thread_getpriority(m_thread->tid, &priority);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, 0);
            return priority;
        }

#endif

        bool operator==(const self &other) const {
            return equal(other);
        }

        bool operator!=(const self &other) const {
            return !equal(other);
        }

    public:
        explicit operator bool() const { return m_thread; }

        operator raw_t *() { return &m_thread->tid; }

        operator raw_t *() const { return &m_thread->tid; }

    private:
        inner::thread_t *m_thread = nullptr;
    };

    namespace thread {
#if UVCXX_SATISFY_VERSION(1, 45, 0)

        inline int getcpu() { return uv_thread_getcpu(); }

#endif
    }
}

#endif //LIBUVCXX_THREAD_H
