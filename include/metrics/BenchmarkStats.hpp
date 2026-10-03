#ifndef BENCHMARK_STATS_HPP
#define BENCHMARK_STATS_HPP

#include "LatencyTracker.hpp"

#include <cstdint>

namespace trading {

class BenchmarkStats {
public:
    BenchmarkStats();

    void addLatency(std::uint64_t latency);

    void addEvent();

    void addOrder();

    std::uint64_t eventCount() const;

    std::uint64_t orderCount() const;

    const LatencyTracker& latency() const;

    double eventsPerSecond(
        double elapsed_seconds
    ) const;

    double ordersPerSecond(
        double elapsed_seconds
    ) const;

    double averageNanosecondsPerEvent() const;

private:
    std::uint64_t event_count_;
    std::uint64_t order_count_;

    LatencyTracker latency_tracker_;
};

} // namespace trading

#endif