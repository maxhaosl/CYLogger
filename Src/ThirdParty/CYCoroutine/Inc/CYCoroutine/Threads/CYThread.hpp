#ifndef __CY_THREAD_CORO_HPP__
#define __CY_THREAD_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"

#include <functional>
#include <string_view>
#include <thread>

CYCOROUTINE_NAMESPACE_BEGIN

class CYCOROUTINE_API CYThread
{
public:
    CYThread() noexcept = default;
    CYThread(CYThread&&) noexcept = default;

    CYThread& operator=(CYThread&& rhs) noexcept = default;
    virtual ~CYThread() = default;

public:
    template<class CALLABLE_TYPE>
    CYThread(std::string strName, CALLABLE_TYPE&& callable, FuncThreadDelegate funStartedCallBack, FuncThreadDelegate funTerminatedCallBack)
    {
        m_thread = std::jthread([strName = std::move(strName), callable = std::forward<CALLABLE_TYPE>(callable), funStartedCallBack = std::move(funStartedCallBack), funTerminatedCallBack = std::move(funTerminatedCallBack)]() mutable {

            SetName(strName);

            if (static_cast<bool>(funStartedCallBack))
            {
                funStartedCallBack(strName);
            }

            callable();

            if (static_cast<bool>(funTerminatedCallBack))
            {
                funTerminatedCallBack(strName);
            }
            });
    }

    std::jthread::id GetId() const noexcept;

    static std::uintptr_t GetVirtualId() noexcept;

    bool Joinable() const noexcept;
    void Join();

    static size_t NumberOfCpu() noexcept;

private:
    std::jthread m_thread;
    static void SetName(std::string_view strName) noexcept;
};

CYCOROUTINE_NAMESPACE_END

#endif // __CY_THREAD_CORO_HPP__
