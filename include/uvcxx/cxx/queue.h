//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_QUEUE_H
#define LIBUVCXX_QUEUE_H

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace uvcxx {
    template<typename... T>
    class queue_core {
    public:
        using value_t = std::tuple<T...>;
        using flag_t = bool;

        bool writable() const {
            auto v = this->m_limit.load();
            return v <= 0 || m_queue.size() < 0;
        }

        bool readable() const {
            return !m_queue.empty();
        }


        bool push(const T &...v) {
            return this->push(value_t(std::make_tuple(v...)));
        }

        bool push(value_t v) {
            std::unique_lock _lock(m_mutex);
            // closed queue can not push more values
            if (m_closed) return false;
            while (true) {
                if (writable()) break;
                m_cond_writable.wait(_lock);
                // pending value will flush to queue (no blocking)
                if (m_closed) break;
            }
            m_queue.push(std::move(v));
            m_cond_readable.notify_one();
            return true;
        }

        std::pair<bool, value_t> pop() {
            std::unique_lock _lock(m_mutex);
            while (true) {
                if (readable()) break;
                if (m_closed) return {false, {}};
                m_cond_readable.wait(_lock);
            }
            auto v = m_queue.front();
            m_queue.pop();
            if (writable()) m_cond_writable.notify_one();
            return {true, v};
        }

        void close() {
            std::unique_lock _lock(m_mutex);
            m_closed = true;
            m_cond_writable.notify_all();
            m_cond_readable.notify_all();
        }

        void limit(int64_t v) {
            std::unique_lock _lock(m_mutex);
            auto pre = m_limit.load();
            if (v <= 0) {
                if (pre > 0) m_cond_writable.notify_all();
            } else if (pre <= 0 || v <= pre) {
                return;
            } else if (v - pre == 1) {
                m_cond_writable.notify_one();
            } else {
                m_cond_writable.notify_all();
            }
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_cond_writable;
        std::condition_variable m_cond_readable;
        std::queue<value_t> m_queue;
        std::atomic<int64_t> m_limit{-1};
        std::atomic<bool> m_closed{false};
    };

    template<typename... T>
    class queue {
    public:
        using self = queue;
        using value_t = std::tuple<T...>;

        class iterator_t {
        public:
            using self = iterator_t;
            using value_t = std::tuple<T...>;

            using value_type = value_t;
            using difference_type = std::ptrdiff_t;
            using pointer = const value_t *;
            using reference = const value_t &;
            using iterator_category = std::forward_iterator_tag;

            bool operator==(const self &) const { return !m_item.first; }

            bool operator!=(const self &) const { return m_item.first; }

            const value_t &operator*() { return m_item.second; }

            const value_t *operator->() { return &m_item.second; }

            self &operator++() { return next(); }

            self operator++(int) { return self(*this).next(); }

        private:
            std::pair<bool, value_t> m_item;
            std::shared_ptr<queue_core<T...>> m_queue;

            explicit iterator_t(std::shared_ptr<queue_core<T...>> queue) : m_queue(std::move(queue)) {}

            self &next() {
                m_item = m_queue->pop();
                return *this;
            }

            friend class queue;
        };

        queue() : m_core(std::make_shared<queue_core<T...>>()) {}

        bool push(const T &...v) { return m_core->push(v...); }

        bool push(value_t v) { return m_core->push(std::move(v)); }

        std::pair<bool, value_t> pop() { return m_core->pop(); }

        void close() { m_core->close(); }

        self &limit(int64_t v) {
            m_core->limit(v);
            return *this;
        }

        iterator_t begin() { return iterator_t(m_core).next(); }

        iterator_t end() { return iterator_t(m_core); }

    private:
        std::shared_ptr<queue_core<T...>> m_core;
    };
}

#endif //LIBUVCXX_QUEUE_H
