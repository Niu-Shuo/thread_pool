#ifndef NRPC_BASE_DONE_GUARD_H
#define NRPC_BASE_DONE_GUARD_H

#include <google/protobuf/service.h>

/*
    封装闭包执行逻辑： DoneGuard 类封装了一个指向 Google Protocol Buffers 闭包的指针，它会在对象销毁时执行这个闭包。
    提供构造和析构函数： 构造函数允许传入一个闭包指针作为参数，以初始化 DoneGuard 对象。析构函数会在对象销毁时执行所持有的闭包。
    提供方法操作闭包： release() 方法可以释放闭包而不执行它，reset() 方法可以重置闭包为新的闭包，并在必要时执行先前的闭包。
    禁止复制构造和赋值： 通过将复制构造函数和赋值运算符声明为私有并删除其定义，确保了 DoneGuard 对象不会被复制构造或赋值。
*/

namespace nrpc
{
    // 用于管理在销毁时执行回调（闭包）的类。
    class DoneGuard
    {
    public:
        // 构造函数：用可选闭包初始化 DoneGuard。
        // 参数：
        //   done：指向要执行的 Google Protocol Buffers 闭包的指针。
        DoneGuard(::google::protobuf::Closure *done = nullptr) : m_done(done) {}

        // 析构函数：如果已设置，则执行闭包。
        ~DoneGuard()
        {
            _run();
        }

        // 释放闭包而不执行它。
        void release()
        {
            m_done = nullptr;
        }

        // 重置闭包为新的闭包，如果已设置，则执行之前的闭包。
        // 参数：
        //   done：指向新的 Google Protocol Buffers 闭包的指针。
        void reset(::google::protobuf::Closure *done)
        {
            _run();
            m_done = done;
        }

    private:
        // 如果已设置，则执行闭包。
        void _run()
        {
            if (m_done != nullptr)
            {
                m_done->Run();
                m_done = nullptr;
            }
        }

    protected:
        // 禁止复制构造和赋值。
        DoneGuard(const DoneGuard &that) = delete;
        DoneGuard &operator=(const DoneGuard &that) = delete;

    private:
        // 指向要执行的 Google Protocol Buffers 闭包的指针。
        ::google::protobuf::Closure *m_done;
    };
} // namespace nrpc

#endif