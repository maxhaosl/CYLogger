#ifndef __CY_JTHREAD_DEFINE_HPP__
#define __CY_JTHREAD_DEFINE_HPP__

#if defined(__has_include)
#  if __has_include(<version>)
#    include <version>
#  endif
#endif

#if defined(__cpp_lib_jthread)
#include <thread>
#include <stop_token>

#else
#include <thread>
#include <atomic>
#include <functional>
#include <utility>

namespace std
{
    class jthread
    {
    public:
        using id = thread::id;
        jthread() noexcept = default;

        template<class Function, class... Args>
        explicit jthread(Function&& f, Args&&... args)
        {
            start(std::forward<Function>(f), std::forward<Args>(args)...);
        }

        ~jthread()
        {
            request_stop();
            join();
        }


        jthread(const jthread&) = delete;
        jthread& operator=(const jthread&) = delete;


        jthread(jthread&& other) noexcept
        {
            swap(other);
        }
        jthread& operator=(jthread&& other) noexcept
        {
            if (this != &other)
            {
                request_stop();
                join();
                swap(other);
            }
            return *this;
        }

        // API

        bool joinable() const noexcept { return thread_.joinable(); }

        void join()
        {
            if (thread_.joinable()) thread_.join();
        }

        void request_stop() noexcept
        {
            stop_requested_.store(true, std::memory_order_relaxed);
        }

        bool stop_requested() const noexcept
        {
            return stop_requested_.load(std::memory_order_relaxed);
        }

        // Provide a simple alternative implementation on platforms that do not support stop_token
        struct stop_token
        {
            bool stop_requested() const { return false; }
        };
        
        stop_token get_stop_token() const noexcept
        {
            // Fallback does not support stop_token
            return stop_token{};
        }

        id get_id() const noexcept
        {
            return thread_.get_id();
        }

    private:
        template<class Function, class... Args>
        void start(Function&& f, Args&&... args)
        {
            stop_requested_ = false;

            thread_ = std::thread(
                [this, func = std::forward<Function>(f)]
                (Args... params) mutable {
                    func(*this, std::forward<Args>(params)...);
                },
                std::forward<Args>(args)...
            );
        }

        void swap(jthread& other) noexcept
        {
            std::swap(thread_, other.thread_);
            bool temp = stop_requested_.load();
            stop_requested_.store(other.stop_requested_.load());
            other.stop_requested_.store(temp);
        }

    private:
        std::thread thread_;
        std::atomic<bool> stop_requested_{ false };
    };
} // namespace std

#endif

#endif // __CY_JTHREAD_DEFINE_HPP__