//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_DETACH_H
#define LIBUVCXX_DETACH_H

#include <functional>
#include <memory>

namespace uvcxx {
    /**
     * attachment manage, use to handle `attachment` on finalize.
     * it can detach `attachment` so nothing will handle.
     */
    class attach_core_t {
    public:
        using attachment_t = std::function<void(void)>;

        attach_core_t(const attach_core_t &) = delete;

        attach_core_t &operator=(const attach_core_t &) = delete;

        explicit attach_core_t(attachment_t attachment)
                : m_attachment(std::move(attachment)) {}

        ~attach_core_t() {
            if (m_attachment) m_attachment();
        }

        void detach() {
            m_attachment = nullptr;
        }

        void attach(attachment_t attachment) {
            m_attachment = std::move(attachment);
        }

    private:
        attachment_t m_attachment;
    };

    class attach_t {
    public:
        using self = attach_t;

        using core_t = attach_core_t;
        using attachment_t = core_t::attachment_t;

        attach_t()
                : m_core(std::make_shared<core_t>(nullptr)) {}

        explicit attach_t(attachment_t attachment)
                : m_core(std::make_shared<core_t>(std::move(attachment))) {}

    protected:
        void _detach_() {
            m_core->detach();
        }

        void _attach_(attachment_t attachment) {
            m_core->attach(std::move(attachment));
        }

        auto _attach_count_() {
            return m_core.use_count();
        }

        /**
         * make this object not use.
         * use to free capture values of lambda function
         * @return
         */
        void _unref_attach_() {
            m_core.reset();
        }

    private:
        std::shared_ptr<attach_core_t> m_core;
    };
}

#endif //LIBUVCXX_DETACH_H
