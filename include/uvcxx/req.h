//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_REQ_H
#define LIBUVCXX_REQ_H

#include <uv.h>

#include "cxx/except.h"
#include "cxx/version.h"
#include "cxx/wrapper.h"
#include "inner/base.h"
#include "utils/defer.h"
#include "utils/promise.h"

namespace uv {
    /**
     * The data field of uv_req_t will be retained and used by the UVCXX.
     * Please do not modify data.
     * The data' resources will be freed after each request action finished.
     * See [lifecycle.md](https://github.com/levalup/libuvcxx/blob/master/docs/lifecycle.md) for more details.
     */
    class req_t : public uvcxx::shared_raw_base_t<uv_req_t> {
    public:
        using self = req_t;
        using supper = uvcxx::shared_raw_base_t<uv_req_t>;

        using supper::supper;

        UVCXX_NODISCARD
        void *data() const {
            return raw<raw_t>()->data;
        }

        UVCXX_NODISCARD
        uv_req_type type() const {
            return raw<raw_t>()->type;
        }

        int cancel() {
            UVCXX_PROXY(uv_cancel(*this));
        }

        UVCXX_NODISCARD
        size_t size() const {
            return uv_req_size(raw()->type);
        }

        UVCXX_NODISCARD
        uv_req_type get_type() const {
            // cover uv_req_get_type
            return raw()->type;
        }

#if UVCXX_SATISFY_VERSION(1, 19, 0)

        UVCXX_NODISCARD
        const char *type_name() const {
            return uv_req_type_name(raw()->type);
        }

#endif

        UVCXX_NODISCARD
        void *get_data() const {
            // cover uv_req_get_data
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
            // cover uv_req_set_data
            raw()->data = data;
        }

    public:
        /**
         * the uv_req_t->data must be the sub-class of data_t
         */
        class data_t {
        public:
            static constexpr uint64_t MAGIC = 0x554332055443320;
            uint64_t magic = MAGIC;

            static inline bool is_it(void *data) {
                return data && ((data_t *) data)->magic == MAGIC;
            }

        public:
            explicit data_t(const req_t &req)
                    : m_req(req.shared_raw()) {
                if (m_req->data) throw uvcxx::errcode(UV_EPERM, "duplicated data initialization");
            }

            virtual ~data_t() = default;

            raw_t *req() { return m_req.get(); }

            UVCXX_NODISCARD
            const raw_t *req() const { return m_req.get(); }

            template<typename T>
            T *req() { return (T *) m_req.get(); }

            template<typename T>
            UVCXX_NODISCARD
            const T *req() const { return (const T *) m_req.get(); }

        private:
            // store the instance of `req` to avoid resource release caused by no external reference
            std::shared_ptr<raw_t> m_req;
        };

    protected:
        static inline self borrow(raw_t *raw) {
            return self{borrow_t(raw)};
        }
    };

    template<typename T, typename B, typename=typename std::enable_if<std::is_base_of<req_t, B>::value>::type>
    class inherit_req_t : public B {
    public:
        using self = inherit_req_t;
        using supper = B;
        using raw_t = T;

        inherit_req_t()
                : supper(make_shared()) {
            this->set_data(nullptr);
        }

        operator T *() { return this->template raw<T>(); }

        operator T *() const { return this->template raw<T>(); }

    private:
        static inline std::shared_ptr<uv_req_t> make_shared() {
            auto r = std::make_shared<T>();
            auto p = reinterpret_cast<typename std::shared_ptr<uv_req_t>::element_type *>(r.get());
            return std::shared_ptr<uv_req_t>{r, p};
        }
    };

    template<typename REQ, typename... ARGS>
    class req_callback_t : public req_t::data_t {
    public:
        using self = req_callback_t;
        using supper = req_t::data_t;

        using supper::supper;

        virtual uvcxx::promise_proxy<REQ *, ARGS...> &proxy() UVCXX_NOEXCEPT = 0;

        virtual void finalize(REQ *req, ARGS... args) UVCXX_NOEXCEPT = 0;

        virtual int check(REQ *req, ARGS... args) UVCXX_NOEXCEPT = 0;

        static void raw_callback(REQ *req, ARGS... args) {
            auto data = (self *) (req->data);
            if (!data) return;

            // set `data` to `nullptr` so that `req` can be reused in the promise's callback.
            req->data = nullptr;

            uvcxx::defer delete_data(std::default_delete<self>(), data);
            // uvcxx::defer finalize_data([&]() { data->finalize(req, args...); }); //< done this in following codes

            auto &proxy = data->proxy();
            uvcxx::defer promise_finally([&]() { proxy.finalize(); });

            try {
                auto status = data->check(req, args...);
                // cancel request will not do anything
                if (status == UV_ECANCELED) return;
                if (status < 0) {
                    (void) proxy.template reject<uvcxx::errcode>(status);
                } else {
                    proxy.resolve(req, args...);
                }
            } catch (...) {
                (void) proxy.reject(std::current_exception());
            }

            // for gcc 4.8.x not support well about use variadic templates in lambda
            // is ok not using defer, as proxy.reject will never throw exception
            data->finalize(req, args...);
        }
    };
}

#endif //LIBUVCXX_REQ_H
