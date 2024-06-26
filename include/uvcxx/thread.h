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

        public:
            raw_t tid{};

        private:
            entry_t m_entry;
            std::atomic<bool> m_joined{false};
            bool m_detached = false;

            static void raw_entry(void *arg) {
                try {
                    ((self *) arg)->m_entry();
                } catch (...) {
                }
            }

        public:
            static thread_t *detach(raw_t id) {
                return new self(id);
            }

            UVCXX_NODISCARD
            bool detached() const { return m_detached; }

        private:
            explicit thread_t(raw_t id)
                    : tid(id), m_detached(true) {
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

        thread_t(thread_t &&that) UVCXX_NOEXCEPT {
            this->operator=(std::move(that));
        }

        thread_t &operator=(thread_t &&that) UVCXX_NOEXCEPT {
            std::swap(m_thread, that.m_thread);
            return *this;
        }

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
            if (m_thread && !m_thread->detached()) return m_thread->join();
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

        UVCXX_NODISCARD
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

        UVCXX_NODISCARD
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

    public:
        static self detach(uv_thread_t tid) { return self{tid}; }

    private:
        explicit thread_t(uv_thread_t tid)
                : m_thread(inner_t::detach(tid)) {}
    };

    namespace thread {
#if UVCXX_SATISFY_VERSION(1, 45, 0)

        inline int getcpu() { return uv_thread_getcpu(); }

#endif

        /**
         * The return thread is marked as `detached`, so using join on it has no effect.
         * @return
         */
        inline thread_t self() {
            return thread_t::detach(uv_thread_self());
        }
    }
}

#endif //LIBUVCXX_THREAD_H
