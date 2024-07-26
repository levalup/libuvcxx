//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CLIP_H
#define LIBUVCXX_CLIP_H

#include <functional>
#include <memory>

namespace uvcxx {
    /**
     * clip manage, use to handle `paper` on finalize.
     * it can detach `handle` so nothing will do after handle finalize.
     */
    class clip_core_t {
    public:
        using paper_t = std::function<void(void)>;

        clip_core_t(const clip_core_t &) = delete;

        clip_core_t &operator=(const clip_core_t &) = delete;

        explicit clip_core_t(paper_t paper)
                : m_paper(std::move(paper)) {}

        ~clip_core_t() {
            if (m_paper) m_paper();
        }

        void detach() {
            m_paper = nullptr;
        }

        void attach(paper_t paper) {
            m_paper = std::move(paper);
        }

    private:
        paper_t m_paper;
    };

    class clip_t {
    public:
        using self = clip_t;

        using core_t = clip_core_t;
        using paper_t = core_t::paper_t;

        clip_t(std::nullptr_t) {}

        clip_t() : m_core(std::make_shared<core_t>(nullptr)) {}

        explicit clip_t(paper_t paper)
                : m_core(std::make_shared<core_t>(std::move(paper))) {}

        void detach() {
            m_core->detach();
        }

        void attach(paper_t paper) {
            m_core->attach(std::move(paper));
        }

    private:
        std::shared_ptr<clip_core_t> m_core;
    };
}

#endif //LIBUVCXX_CLIP_H
