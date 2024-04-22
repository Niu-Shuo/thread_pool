#ifndef NRPC_BASE_DONE_GUARD_H
#define NRPC_BASE_DONE_GUARD_H

#include <google/protobuf/service.h>

namespace nrpc
{
    class DoneGuard
    {
    public:
        DoneGuard(::google::protobuf::Closure *done = nullptr) : m_done(done) {}
        ~DoneGuard()
        {
            _run();
        }

        void release()
        {
            m_done = nullptr;
        }

        void reset(::google::protobuf::Closure *done)
        {
            _run();
            m_done = done;
        }

    private:
        void _run()
        {
            if (m_done != nullptr)
            {
                m_done->Run();
                m_done = nullptr;
            }
        }

    protected:
        DoneGuard(const DoneGuard &that) = delete;
        DoneGuard &operator=(const DoneGuard &that) = delete;

    private:
        ::google::protobuf::Closure *m_done;
    };
} // namespace nrpc

#endif