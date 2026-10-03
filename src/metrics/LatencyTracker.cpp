#include "../../include/metrics/LatencyTracker.hpp"

#include <algorithm>
#include <numeric>

namespace trading {

LatencyTracker::LatencyTracker(
    std::size_t reserve_samples)
{
    /*
     * Reserving storage upfront avoids repeated vector
     * reallocations when the expected workload is known.
     */
    if (reserve_samples > 0) {
        samples_.reserve(reserve_samples);
    }
}

void LatencyTracker::record(
    TimePoint start,
    TimePoint end)
{
    samples_.push_back(
        elapsedNanoseconds(start, end)
    );
}

void LatencyTracker::recordNanoseconds(
    std::uint64_t latency)
{
    samples_.push_back(latency);
}

std::size_t LatencyTracker::count() const
{
    return samples_.size();
}

std::uint64_t LatencyTracker::minLatency() const
{
    if (samples_.empty()) {
        return 0;
    }

    return *std::min_element(
        samples_.begin(),
        samples_.end()
    );
}

std::uint64_t LatencyTracker::maxLatency() const
{
    if (samples_.empty()) {
        return 0;
    }

    return *std::max_element(
        samples_.begin(),
        samples_.end()
    );
}

double LatencyTracker::averageLatency() const
{
    if (samples_.empty()) {
        return 0.0;
    }

    const std::uint64_t total =
        std::accumulate(
            samples_.begin(),
            samples_.end(),
            std::uint64_t{0}
        );

    return static_cast<double>(total) /
           static_cast<double>(samples_.size());
}

std::uint64_t LatencyTracker::percentile(
    double p) const
{
    if (samples_.empty()) {
        return 0;
    }

    /*
     * Clamp the percentile to the valid range.
     */
    if (p < 0.0) {
        p = 0.0;
    }

    if (p > 1.0) {
        p = 1.0;
    }

    /*
     * Reporting happens outside the hot path, so making
     * a copy and sorting it keeps record() inexpensive.
     */
    std::vector<std::uint64_t> sorted =
        samples_;

    std::sort(
        sorted.begin(),
        sorted.end()
    );

    const double position =
        p * static_cast<double>(
            sorted.size() - 1
        );

    const std::size_t index =
        static_cast<std::size_t>(position);

    return sorted[index];
}

void LatencyTracker::clear()
{
    samples_.clear();
}

} // namespace trading