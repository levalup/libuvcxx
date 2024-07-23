//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_ATTACHED_CALLBACK_H
#define LIBUVCXX_ATTACHED_CALLBACK_H

#include "callback.h"
#include "detach.h"

namespace uvcxx {
    template <typename... Args>
    class attached_callback : public callback<Args...> {
    public:
        using self = attached_callback;
        using supper = callback<Args...>;

        attached_callback(attach_t attach, supper cb)
                : supper(std::move(cb)), m_attach(std::move(attach)) {
        }

        callback<Args...> &detach() {
            m_attach.detach();
            return *this;
        }

    private:
        uvcxx::attach_t m_attach;
    };
}

#endif //LIBUVCXX_ATTACHED_CALLBACK_H
