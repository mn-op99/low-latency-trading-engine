#include "../include/queue/SPSCQueue.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>

int main()
{
    using namespace trading;

    constexpr std::size_t iterations = 1'000'000;

    SPSCQueue<std::uint64_t, 4096> queue;

    std::uint64_t value = 0;

    const auto start = std::chrono::steady_clock::now();

    std::size_t pushed = 0;
    std::size_t popped = 0;

    for (std::size_t i = 0; i < iterations; ++i) {
        if (queue.push(i)) {
            ++pushed;
        }

        if (queue.pop(value)) {
            ++popped;
        }
    }

    const auto end = std::chrono::steady_clock::now();

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start
        ).count();

    std::cout << "SPSC Queue Benchmark\n";
    std::cout << "--------------------\n";
    std::cout << "Iterations : " << iterations << '\n';
    std::cout << "Pushed     : " << pushed << '\n';
    std::cout << "Popped     : " << popped << '\n';
    std::cout << "Time (ns)  : " << elapsed << '\n';

    if (pushed > 0) {
        std::cout << "Avg (ns/op): "
                  << static_cast<double>(elapsed) / pushed
                  << '\n';
    }

    return 0;
}