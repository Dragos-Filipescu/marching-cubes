#pragma once
#ifndef MARCHING_CUBES_UTILS_FRAME_STATS_HPP
#define MARCHING_CUBES_UTILS_FRAME_STATS_HPP

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <concepts>
#include <format>
#include <ostream>
#include <ratio>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <core/aliases.hpp>

namespace marching_cubes::utils::frame_stats {

    namespace detail {

        template<typename T>
		concept Arithmetic = std::is_arithmetic_v<T>;
    }

    template<detail::Arithmetic T>
    struct BasicStatsSnapshot final {

        using ValueType = T;

        T avg{};
        T min{};
        T max{};
        T p99{};
        T p999{};
    };

	using FrameStatsSnapshot = BasicStatsSnapshot<f32>;

    template <detail::Arithmetic T, std::size_t History = 1024>
    class BasicStatsCollector final {
    public:

		using ValueType = T;
		using SnapshotType = BasicStatsSnapshot<T>;

        void addSample(T value)
        {
            m_Buf[m_Head] = value;
            m_Head = (m_Head + 1) % m_Buf.size();
            if (m_Count < m_Buf.size()) {
                ++m_Count;
            }
        }

        BasicStatsSnapshot<T> snapshot() const
        {
            BasicStatsSnapshot<T> s{};
            if (m_Count == 0) {
                return s;
            }

            std::vector<T> v;
            v.reserve(m_Count);
            for (size_t i = 0; i < m_Count; ++i) {
                size_t idx = (m_Head + m_Buf.size() - m_Count + i) % m_Buf.size();
                v.push_back(m_Buf[idx]);
            }

            // mean/min/max
            T sum = 0, mn = v[0], mx = v[0];
            for (T x : v) {
                sum += x;
                mn = std::min(mn, x);
                mx = std::max(mx, x);
            }
            s.avg = sum / v.size();
            s.min = mn;
            s.max = mx;

            auto percentile = [&](f32 p) -> T {
                if (v.empty()) {
                    return 0;
                }
                f32 fp = p * (v.size() - 1);
                std::size_t k = static_cast<std::size_t>(std::floor(fp));
                std::nth_element(v.begin(), v.begin() + k, v.end());
                return v[k];
            };
            s.p99 = percentile(0.99f);
            s.p999 = percentile(0.999f);

            return s;
        }

    private:
        std::array<T, History> m_Buf{};
        std::size_t m_Head = 0;
        std::size_t m_Count = 0;
    };

    class FrameStats {
    public:
        using ClockType = std::chrono::steady_clock;

        explicit FrameStats(
            std::chrono::seconds reportInterval = std::chrono::seconds{ 1 }
        )
            : m_ReportInterval{ reportInterval },
            m_LastReport{ ClockType::now() }
        {
        }

        void addFrame(ClockType::time_point now, ClockType::time_point prev)
        {
            f32 ms = std::chrono::duration<f32, std::milli>(now - prev).count();
            // overwrite ring buffer
			m_Collector.addSample(ms);
        }

        bool ready(ClockType::time_point now) const
        {
            return (now - m_LastReport) >= m_ReportInterval;
        }

        FrameStatsSnapshot snapshotAndBump(ClockType::time_point now)
        {
            m_LastReport = now;
            return m_Collector.snapshot();
        }

    private:
        BasicStatsCollector<f32> m_Collector{};
        std::chrono::seconds m_ReportInterval;
        ClockType::time_point m_LastReport;
    };

    namespace detail {

        inline std::string toString(const FrameStatsSnapshot& s)
        {
			auto fpsAvg = (s.avg > 0.0f ? 1000.0f / s.avg : 0.0f);
			auto fps1 = (s.p99 > 0.0f ? 1000.0f / s.p99 : 0.0f);
			auto fps01 = (s.p999 > 0.0f ? 1000.0f / s.p999 : 0.0f);
            return std::format(
                "FPS avg {:7.1f} fps | 1% low {:7.1f} fps | 0.1% low {:7.1f} fps | "
                "FT avg {:7.2f} ms | p99 {:7.2f} ms | p99.9 {:7.2f} ms | min {:7.2f} ms / max {:7.2f} ms",
                fpsAvg, fps1, fps01,
                s.avg, s.p99, s.p999, s.min, s.max
            );
        }
    }

    inline std::ostream& operator<<(std::ostream& os, const FrameStatsSnapshot& s)
    {
        return os << detail::toString(s);
    }
}

template <>
struct std::formatter<marching_cubes::utils::frame_stats::FrameStatsSnapshot> {
    // style: 'v' verbose (default), 'f' fps-only, 'm' ms-only, 'c' compact
    char style = 'v';

    constexpr auto parse(std::format_parse_context& ctx)
    {
        auto it = ctx.begin();
        auto end = ctx.end();
        if (it != end && (*it == 'v' || *it == 'f' || *it == 'm' || *it == 'c')) {
            style = *it++;
        }
        if (it != end && *it != '}') {
            throw std::format_error{ "Invalid format for FrameStatsSnapshot." };
        }
        return it;
    }

    template <typename FormatContext>
    auto format(
        const marching_cubes::utils::frame_stats::FrameStatsSnapshot& s,
        FormatContext& ctx
    ) const
    {
        using namespace marching_cubes::utils::frame_stats;
        return std::format_to(ctx.out(), "{}", detail::toString(s));
    }
};

#endif // !MARCHING_CUBES_UTILS_FRAME_STATS_HPP