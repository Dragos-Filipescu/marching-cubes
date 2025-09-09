#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <exception>
#include <future>
#include <latch>
#include <memory>
#include <numeric>
#include <optional>
#include <print>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

#include <utils/thread_pool.hpp>

using marching_cubes::utils::threading::ThreadPool;
using namespace std::chrono;

// -----------------------
// helpers
// -----------------------

namespace {

    void banner(const char* title)
    {
        std::println("\n============================================");
        std::println("=== {} ===", title);
        std::println("============================================");
    }

    // Returns true if submit-after-stop correctly rejects:
    //   - either by throwing, or by returning an invalid future.
    template <typename F>
    bool expectSubmitRejected(ThreadPool& pool, F&& f)
    {
        try {
            auto fut = pool.submitFuture(std::forward<F>(f));
            if (!fut) {
                return true;
            }
            return false;
        }
        catch (const std::exception& e) {
            std::println("  [submit rejected] threw as expected: {}", e.what());
            return true;
        }
        catch (...) {
            std::println("  [submit rejected] threw (unknown) as expected");
            return true;
        }
    }

    // -----------------------
    // tests
    // -----------------------

    // 1) basic submit/get
    void testBasicSubmit()
    {
        banner("Test 1: Basic submit/get");

        ThreadPool pool{ 2 };

        auto f1 = pool.submitFuture([](std::stop_token) { return 123; });
        auto f2 = pool.submitFuture([](std::stop_token) { return std::string("hello"); });
        auto f3 = pool.submitFuture([](std::stop_token) { std::this_thread::sleep_for(5ms); });

        assert(f1->get() == 123);
        assert(f2->get() == "hello");
        f3->get(); // void

        std::println("  basic submit/get passed");
    }

    // 2) parallelism degree (rough)
    void testParallelismDegree()
    {
        banner("Test 2: Parallelism degree");

        using namespace std::chrono;

        constexpr int kThreads = 4;
        constexpr int kTasks = 8;
        constexpr auto kWork = 50ms;

        ThreadPool pool{ kThreads };

        // Start gate (released by main once all tasks are queued)
        std::latch start{ 1 };

        // Finish gate (wait for all tasks)
        std::latch done{ kTasks };

        for (int i = 0; i < kTasks; ++i) {
            auto _ = pool.submitFuture(
                [&](std::stop_token) {
                    start.wait();                // wait until main releases
                    std::this_thread::sleep_for(kWork);
                    done.count_down();           // signal finished
                }
            );
            // optional: check _.has_value() in case submit can reject
        }

        const auto t0 = steady_clock::now();
        start.count_down();  // release all workers simultaneously
        done.wait();         // wait for all tasks to finish
        const auto t1 = steady_clock::now();

        const auto elapsed_ms = duration_cast<milliseconds>(t1 - t0).count();
        std::println("  elapsed ~ {} ms for {} tasks on {} threads ({} ms each)",
            elapsed_ms, kTasks, kThreads, kWork.count());

        // Expect ~ceil(kTasks / kThreads) waves: 8/4 = 2 → ~2 * 50ms = ~100ms (+ overhead)
        const auto expected = 2 * kWork;
        assert(elapsed_ms >= (expected - 10ms).count()); // small tolerance
        std::println("  parallelism degree passed");
    }

    // 3) many tasks + atomic accumulation
    void testManyTasksAccumulate()
    {
        banner("Test 3: Many tasks & accumulation");

        ThreadPool pool{ std::max(2u, std::thread::hardware_concurrency()) };

        constexpr int N = 1000;
        std::atomic<int> counter{ 0 };

        std::vector<std::optional<std::future<void>>> fs{};
        fs.reserve(N);
        for (int i = 0; i < N; ++i) {
            fs.emplace_back(pool.submitFuture([&counter](std::stop_token) { counter.fetch_add(1, std::memory_order_relaxed); }));
        }
        for (auto& f : fs) {
            f->get();
        }

        std::println("  counter = {} (expected {})", counter.load(), N);
        assert(counter.load() == N);
        std::println("  many tasks accumulation passed");
    }

    // 4) exception propagation through future
    void testExceptionPropagation()
    {
        banner("Test 4: Exception propagation");

        ThreadPool pool{ 2 };

        auto f = pool.submitFuture([](std::stop_token) -> int {
            throw std::runtime_error{ "boom" };
            });

        bool threw = false;
        try {
            (void)f->get();
        }
        catch (const std::runtime_error& e) {
            std::println("  caught std::runtime_error: {}", e.what());
            threw = true;
        }
        catch (...) {
            threw = true;
        }
        assert(threw);
        std::println("  exception propagation passed");
    }

    // 5) move-only captures support
    void testMoveOnlyTask()
    {
        banner("Test 5: Move-only capture");

        ThreadPool pool{ 2 };

        auto fut = pool.submitFuture([p = std::make_unique<int>(21)](std::stop_token) mutable {
            return *p * 2;
            });

        assert(fut->get() == 42);
        std::println("  move-only capture passed");
    }

    // 6) graceful stop/shutdown behavior
    void testStopRejectsSubmit()
    {
        banner("Test 6: Stop/shutdown rejects submit");

        ThreadPool pool{ 2 };

        // Run a couple of tasks just to touch the workers
        auto a = pool.submitFuture([](std::stop_token) { return 1; });
        auto b = pool.submitFuture([](std::stop_token) { return 2; });
        assert(a->get() == 1);
        assert(b->get() == 2);

        // stop/shutdown the pool
        pool.stop(); // (or pool.shutdown(); depending on your API)

        // Now submitting should not be accepted. Different APIs either:
        //  - throw, or
        //  - return an invalid future.
        bool ok = expectSubmitRejected(pool, [](std::stop_token) { return 3; });
        assert(ok && "submit after stop should be rejected");

        std::println("  stop/shutdown behavior passed");
    }

    // 7) FIFO-ish completion (NOT strict guarantee, just smoke test)
    // We just check all results arrived; we don't enforce ordering since
    // a real pool should not guarantee it.
    void testBulkResultsIntegrity()
    {
        banner("Test 7: Bulk results integrity");

        ThreadPool pool{ 4 };
        constexpr int N = 512;

        std::vector<std::optional<std::future<int>>> fs{};
        fs.reserve(N);
        for (int i = 0; i < N; ++i) {
            int v = i;
            fs.emplace_back(pool.submitFuture([v](std::stop_token) { return v * v; }));
        }

        std::vector<char> seen(N, 0);
        for (auto& f : fs) {
            int x = f->get();
            int r = static_cast<int>(std::sqrt(x) + 0.5);
            assert(r >= 0 && r < N);
            assert(r * r == x);
            seen[r] = 1;
        }
        const int covered = std::accumulate(seen.begin(), seen.end(), 0);
        std::println("  covered {} / {} results", covered, N);
        assert(covered == N);

        std::println("  bulk results integrity passed");
    }
}

namespace marching_cubes::tests {

    // -----------------------
    // Public runner
    // -----------------------
    void runThreadPoolTests()
    {
        banner("Running ThreadPool Unit Tests");

        testBasicSubmit();
        testParallelismDegree();
        testManyTasksAccumulate();
        testExceptionPropagation();
        testMoveOnlyTask();
        testStopRejectsSubmit();
        testBulkResultsIntegrity();

        banner("All ThreadPool tests passed!");
    }

} // namespace marching_cubes::tests
