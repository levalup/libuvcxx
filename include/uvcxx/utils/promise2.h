//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_PROMISE2_H
#define LIBUVCXX_PROMISE2_H

#include "promise.h"
#include "clip.h"

namespace uvcxx {
    template <typename... Args>
    class promise2 : public promise<Args...> {
    public:
        using self = promise2;
        using supper = promise<Args...>;

        promise2(clip_t clip, supper cb)
                : supper(std::move(cb)), m_clip(std::move(clip)) {
        }

        promise<Args...> &detach() {
            m_clip.detach();
            return *this;
        }

    private:
        uvcxx::clip_t m_clip;
    };
}

#endif //LIBUVCXX_PROMISE2_H
