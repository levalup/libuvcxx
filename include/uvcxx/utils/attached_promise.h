//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_ATTACHED_PROMISE_H
#define LIBUVCXX_ATTACHED_PROMISE_H

#include "promise.h"
#include "detach.h"

namespace uvcxx {
    template <typename... Args>
    class attached_promise : public promise<Args...> {
    public:
        using self = attached_promise;
        using supper = promise<Args...>;

        attached_promise(attach_t attach, supper cb)
                : supper(std::move(cb)), m_attach(std::move(attach)) {
        }

        promise<Args...> &detach() {
            m_attach.detach();
            return *this;
        }

    private:
        uvcxx::attach_t m_attach;
    };
}

#endif //LIBUVCXX_ATTACHED_PROMISE_H
