#ifndef LATENCY_TRACKER_HPP
#define LATENCY_TRACKER_HPP

#include "../common/TimeUtils.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace trading {

class LatencyTracker {
public:
    explicit LatencyTracker(
        std::size_t reserve_samples = 0
    );

    /*
     * Record latency directly from two timestamps.
     */
    void record(
        TimePoint start,
        TimePoint end
    );

    /*
     * Record an already measured latency.
     */
    void recordNanoseconds(
        std::uint64_t latency
    );

    std::size_t count() const;

    std::uint64_t minLatency() const;

    std::uint64_t maxLatency() const;

    double averageLatency() const;

    /*
     * p is expressed as a fraction:
     *
     *   0.50  -> p50
     *   0.95  -> p95
     *   0.99  -> p99
     *   0.999 -> p99.9
     */
    std::uint64_t percentile(
        double p
    ) const;

    void clear();

private:
    std::vector<std::uint64_t> samples_;
};

} // namespace trading

#endif