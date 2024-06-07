//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_REQ_H
#define LIBUVCXX_REQ_H

#include <uv.h>

#include "cxx/defer.h"
#include "cxx/except.h"
#include "cxx/promise.h"

namespace uv {
    /**
     * The data field of uv_req_t will be retained and used by the UVCXX.
     * Please do not modify data.
     * The data' resources will be freed after each request action finished.
     */
    class req_t {
    public:
        using self = req_t;
        using raw_t = uv_req_t;

        /**
         * the uv_req_t->data must be the sub-class of data_t
         */
        class data_t {
        public:
            explicit data_t(const req_t &req)
                    : m_req(req.m_raw) {
                m_req->data = this;
            }

            virtual ~data_t() {
                m_req->data = nullptr;
            };

            raw_t *req() { return m_req.get(); }

            [[nodiscard]]
            const raw_t *req() const { return m_req.get(); }

            template<typename T>
            T *req() { return (T *) m_req.get(); }

            template<typename T>
            [[nodiscard]]
            const T *req() const { return (const T *) m_req.get(); }

        private:
            // store the instance of `req` to avoid resource release caused by no external reference
            std::shared_ptr<raw_t> m_req;
        };

        [[nodiscard]]
        void *data() const {
            return raw<raw_t>()->data;
        }

        [[nodiscard]]
        uv_req_type type() const {
            return raw<raw_t>()->type;
        }

        int cancel() {
            auto err = uv_cancel(*this);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        [[nodiscard]]
        size_t size() const {
            return uv_req_size(m_raw->type);
        }

        [[nodiscard]]
        void *get_data() const {
            return uv_req_get_data(*this);
        }

        /**
         * Never use this method on your own as it may result in failure.
         * @param data
         */
        void set_data(void *data) {
            uv_req_set_data(*this, data);
        }

        [[nodiscard]]
        uv_req_type get_type() const {
            return uv_req_get_type(*this);
        }

        [[nodiscard]]
        const char *type_name() const {
            return uv_req_type_name(m_raw->type);
        }

        template<typename T>
        [[nodiscard]]
        T *data() const {
            return (T *) m_raw->data;
        }

        template<typename T>
        [[nodiscard]]
        T *get_data() const {
            return (T *) m_raw->data;
        }

        operator raw_t *() { return m_raw.get(); }

        operator raw_t *() const { return m_raw.get(); }

        explicit operator bool() { return bool(m_raw); }

        static self borrow(raw_t *raw) {
            return self{std::shared_ptr<raw_t>(raw, [](raw_t *) {})};
        }

    protected:
        explicit req_t(std::shared_ptr<raw_t> raw)
                : m_raw(std::move(raw)) {}

        raw_t *raw() { return m_raw.get(); }

        raw_t *raw() const { return m_raw.get(); }

        template<typename T>
        T *raw() { return (T *) (m_raw.get()); }

        template<typename T>
        T *raw() const { return (T *) (m_raw.get()); }

    private:
        // store the instance of `req` to avoid resource release caused by no external reference
        std::shared_ptr<raw_t> m_raw;
    };

    template<typename T, typename B, typename=typename std::enable_if_t<std::is_base_of_v<req_t, B>>>
    class req_extend_t : public B {
    public:
        using self = req_extend_t;
        using supper = B;
        using raw_t = T;

        req_extend_t()
                : supper(make_shared()) {
            this->set_data(nullptr);
        }

        operator T *() { return this->template raw<T>(); }

        operator T *() const { return this->template raw<T>(); }

    protected:
        explicit req_extend_t(const std::shared_ptr<T> &raw)
                : supper(std::reinterpret_pointer_cast<uv_req_t>(raw)) {}

    private:
        static std::shared_ptr<uv_req_t> make_shared() {
            return std::reinterpret_pointer_cast<uv_req_t>(std::make_shared<T>());
        }
    };

    template<typename REQ, typename... ARGS>
    class req_callback_t : public req_t::data_t {
    public:
        using self = req_callback_t;
        using supper = req_t::data_t;

        using supper::supper;

        virtual uvcxx::promise_proxy<REQ *, ARGS...> &proxy() noexcept = 0;

        virtual void finalize(REQ *req, ARGS... args) noexcept = 0;

        virtual int check(REQ *req, ARGS... args) noexcept { return 0; }

        static void raw_callback(REQ *req, ARGS... args) {
            auto data = (self *) (req->data);
            if (!data) return;
            uvcxx::defer delete_data(std::default_delete<self>(), data);
            uvcxx::defer finalize_data([&]() { data->finalize(req, args...); });

            auto &proxy = data->proxy();
            uvcxx::defer promise_finally([&]() { proxy.finalize(); });

            try {
                auto err = data->check(req, args...);
                // cancel request will not do anything
                if (err == UV_ECANCELED) return;
                if (err < 0) {
                    proxy.template reject<uvcxx::errcode>(err);
                } else {
                    proxy.resolve(req, args...);
                }
            } catch (...) {
                proxy.reject(std::current_exception());
            }
        }
    };
}

#endif //LIBUVCXX_REQ_H
