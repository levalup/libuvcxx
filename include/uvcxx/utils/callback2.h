//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CALLBACK2_H
#define LIBUVCXX_CALLBACK2_H

#include "callback.h"
#include "clip.h"

namespace uvcxx {
    template <typename... Args>
    class callback2 : public callback<Args...> {
    public:
        using self = callback2;
        using supper = callback<Args...>;

        callback2(clip_t clip, supper cb)
                : supper(std::move(cb)), m_clip(std::move(clip)) {
        }

        callback<Args...> &detach() {
            m_clip.detach();
            return *this;
        }

    private:
        uvcxx::clip_t m_clip;
    };
}

#endif //LIBUVCXX_CALLBACK2_H
