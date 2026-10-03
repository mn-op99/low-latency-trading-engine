#ifndef TIME_UTILS_HPP
#define TIME_UTILS_HPP

#include <chrono>
#include <cstdint>

namespace trading {

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

/*
 * steady_clock is used for latency measurements because
 * it is monotonic and is not affected by system clock
 * adjustments.
 */
inline TimePoint now()
{
    return Clock::now();
}

inline std::uint64_t elapsedNanoseconds(
    TimePoint start,
    TimePoint end)
{
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<
            std::chrono::nanoseconds>(
            end - start
        ).count()
    );
}

} // namespace trading

#endif