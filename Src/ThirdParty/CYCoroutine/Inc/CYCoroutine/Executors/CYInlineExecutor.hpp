#ifndef __CY_INLINE_EXECUTOR_CORO_HPP__
#define __CY_INLINE_EXECUTOR_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Executors/CYExecutor.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

class CYCOROUTINE_API CYInlineExecutor final : public CYExecutor
{
public:
    CYInlineExecutor() noexcept;

public:
    virtual void Enqueue(CYTask task) override;
    virtual void Enqueue(std::span<CYTask> tasks) override;
    virtual int  MaxConcurrencyLevel() const noexcept override;
    virtual void ShutDown() override;
    virtual bool ShutdownRequested() const override;

private:
    void IfAbortThrow() const;

private:
    std::atomic_bool m_bAbort;

};

CYCOROUTINE_NAMESPACE_END

#endif //__CY_INLINE_EXECUTOR_CORO_HPP__
