#include "../../include/metrics/BenchmarkStats.hpp"

namespace trading {

BenchmarkStats::BenchmarkStats()
    : event_count_(0),
      order_count_(0),
      latency_tracker_()
{
}

void BenchmarkStats::addLatency(
    std::uint64_t latency)
{
    latency_tracker_.recordNanoseconds(latency);
}

void BenchmarkStats::addEvent()
{
    ++event_count_;
}

void BenchmarkStats::addOrder()
{
    ++order_count_;
}

std::uint64_t BenchmarkStats::eventCount() const
{
    return event_count_;
}

std::uint64_t BenchmarkStats::orderCount() const
{
    return order_count_;
}

const LatencyTracker& BenchmarkStats::latency() const
{
    return latency_tracker_;
}

double BenchmarkStats::eventsPerSecond(
    double elapsed_seconds) const
{
    if (elapsed_seconds <= 0.0) {
        return 0.0;
    }

    return static_cast<double>(event_count_) /
           elapsed_seconds;
}

double BenchmarkStats::ordersPerSecond(
    double elapsed_seconds) const
{
    if (elapsed_seconds <= 0.0) {
        return 0.0;
    }

    return static_cast<double>(order_count_) /
           elapsed_seconds;
}

double BenchmarkStats::averageNanosecondsPerEvent() const
{
    if (event_count_ == 0) {
        return 0.0;
    }

    /*
     * Use the total number of recorded latency samples
     * when available. Otherwise there is no latency
     * measurement to derive this metric from.
     */
    if (latency_tracker_.count() == 0) {
        return 0.0;
    }

    return latency_tracker_.averageLatency();
}

} // namespace trading