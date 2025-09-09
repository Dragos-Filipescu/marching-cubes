#pragma once
#ifndef MARCHING_CUBES_UTILS_THREADING_THREAD_POOL_HPP
#define MARCHING_CUBES_UTILS_THREADING_THREAD_POOL_HPP

#include <atomic>
#include <concepts>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <optional>
#include <stop_token>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace marching_cubes::utils::threading {

    class ThreadPool final {
    public:

        ThreadPool() noexcept : m_Stopping{ true } {}

        explicit ThreadPool(unsigned int threadCount) noexcept
            : m_Stopping{ false }
        {
            init(threadCount);
        }

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;

        ~ThreadPool() noexcept { stop(); }

        ThreadPool& init(unsigned int threadCount = (std::thread::hardware_concurrency() - 1u)) noexcept
        {
            if (threadCount == 0) {
                threadCount = 1;
            }
            m_Workers.reserve(threadCount);
            for (unsigned int i = 0; i < threadCount; ++i) {
                m_Workers.emplace_back([this](std::stop_token st) { workerLoop(st); });
            }
            m_Stopping = false;
            return *this;
        }

        // Submit fire-and-forget job with stop_token
        template <typename F, typename... Args>
            requires std::invocable<F, std::stop_token, Args...>
        void submit(F&& f, Args&&... args) {
            {
                std::lock_guard lock{ m_Mutex };
                if (m_Workers.empty() || m_Stopping) return;

                m_Tasks.emplace_back(
                    [fn = std::forward<F>(f), ...captured = std::forward<Args>(args)](std::stop_token st) mutable {
                        std::invoke(std::move(fn), st, std::move(captured)...);
                    }
                );
            }
            m_CV.notify_one();
        }

        // Submit job with return value + stop_token propagation
        template <typename F, typename... Args>
            requires std::invocable<F, std::stop_token, Args...>
        [[nodiscard]] auto submitFuture(F&& f, Args&&... args)
            -> std::optional<std::future<std::invoke_result_t<F, std::stop_token, Args...>>>
        {
            using R = std::invoke_result_t<F, std::stop_token, Args...>;

            std::lock_guard lock{ m_Mutex };
            if (m_Workers.empty() || m_Stopping) {
                return std::nullopt;
            }

            auto task = std::packaged_task<R(std::stop_token)>(
                [fn = std::forward<F>(f), ...captured = std::forward<Args>(args)](std::stop_token st) mutable {
                    return std::invoke(std::move(fn), st, std::move(captured)...);
                }
            );
            auto fut = task.get_future();

            m_Tasks.emplace_back([t = std::move(task)](std::stop_token st) mutable { t(st); });
            m_CV.notify_one();
            return fut;
        }

        ThreadPool& start() noexcept
        {
            m_Stopping.store(false, std::memory_order_release);
            return *this;
        }

        ThreadPool& stop() noexcept
        {
            {
                std::scoped_lock lock{ m_Mutex };
                m_Stopping.store(true, std::memory_order_release);
                m_Tasks.clear(); // cancel queued jobs immediately
            }
            m_CV.notify_all();
            for (auto& thrd : m_Workers) {
                thrd.request_stop(); // ask threads to stop
            }
            return *this;
        }

        ThreadPool& waitIdle()
        {
            std::unique_lock lock{ m_Mutex };
            m_CV.wait(lock, [this] {
                return m_Tasks.empty() && m_Inflight.load(std::memory_order_relaxed) == 0;
            });
            return *this;
        }

    private:
        // Each worker executes jobs, passing its own stop_token
        void workerLoop(std::stop_token st)
        {
            while (true) {
                std::move_only_function<void(std::stop_token)> job;
                {
                    std::unique_lock<std::mutex> lock{ m_Mutex };
                    m_CV.wait(lock, [this, &st] {
                        return st.stop_requested() || m_Stopping.load(std::memory_order_acquire) || !m_Tasks.empty();
                        });
                    if ((st.stop_requested() || m_Stopping.load(std::memory_order_acquire)) && m_Tasks.empty()) {
                        return;
                    }
                    job = std::move(m_Tasks.front());
                    m_Tasks.pop_front();
                }
                m_Inflight.fetch_add(1, std::memory_order_relaxed);
                job(st); // propagate stop_token
                m_Inflight.fetch_sub(1, std::memory_order_relaxed);
                m_CV.notify_all();
            }
        }

        std::vector<std::jthread> m_Workers{};
        std::deque<std::move_only_function<void(std::stop_token)>> m_Tasks{};
        std::mutex m_Mutex{};
        std::condition_variable m_CV{};
        std::atomic_bool m_Stopping{ true };
        std::atomic_size_t m_Inflight{ 0 };
    };

} // namespace marching_cubes::utils::threading

#endif // !MARCHING_CUBES_UTILS_THREADING_THREAD_POOL_HPP
