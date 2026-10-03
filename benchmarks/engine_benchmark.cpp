#include "../include/common/Types.hpp"
#include "../include/execution/ExchangeSimulator.hpp"
#include "../include/execution/ExecutionEngine.hpp"
#include "../include/market_data/MarketDataGenerator.hpp"
#include "../include/market_data/OrderBook.hpp"
#include "../include/metrics/LatencyTracker.hpp"
#include "../include/risk/RiskManager.hpp"
#include "../include/strategy/SpreadStrategy.hpp"

#include <chrono>
#include <cstddef>
#include <iostream>

int main()
{
    using namespace trading;

    constexpr InstrumentId near_instrument = 1;
    constexpr InstrumentId far_instrument = 2;

    constexpr std::size_t iterations = 1'000'000;

    /*
     * Components used by the benchmark.
     *
     * The benchmark intentionally measures the core
     * processing pipeline without starting the engine's
     * producer/consumer threads. This gives us a useful
     * single-threaded baseline for the hot path.
     */
    MarketDataGenerator near_generator(
        near_instrument
    );

    MarketDataGenerator far_generator(
        far_instrument
    );

    OrderBook near_book(
        near_instrument
    );

    OrderBook far_book(
        far_instrument
    );

    SpreadStrategy strategy(
        near_instrument,
        far_instrument,
        5,
        10
    );

    RiskManager near_risk_manager(
        100,
        1000
    );

    RiskManager far_risk_manager(
        100,
        1000
    );

    ExchangeSimulator exchange;

    ExecutionEngine execution_engine(
        exchange
    );

    LatencyTracker latency_tracker(
        iterations * 2
    );

    std::size_t generated_trades = 0;
    std::size_t generated_orders = 0;
    std::size_t executed_orders = 0;
    std::size_t executed_trades = 0;

    const auto start =
        std::chrono::steady_clock::now();

    for (std::size_t i = 0; i < iterations; ++i) {

        /*
         * Generate market data for both instruments.
         */
        MarketDataEvent near_event =
            near_generator.nextEvent();

        MarketDataEvent far_event =
            far_generator.nextEvent();

        /*
         * Process both events through their respective
         * order books.
         */
        near_book.update(near_event);
        far_book.update(far_event);

        /*
         * Evaluate the current spread.
         */
        Order near_order{};
        Order far_order{};

        const TimePoint processing_start =
            now();

        const bool signal_generated =
            strategy.onMarketUpdate(
                near_book,
                far_book,
                near_order,
                far_order
            );

        const TimePoint processing_end =
            now();

        latency_tracker.record(
            processing_start,
            processing_end
        );

        if (!signal_generated) {
            continue;
        }

        ++generated_trades;
        generated_orders += 2;

        /*
         * Validate both legs independently.
         */
        if (!near_risk_manager.validate(
                near_order)) {
            continue;
        }

        if (!far_risk_manager.validate(
                far_order)) {
            continue;
        }

        /*
         * Execute both legs.
         */
        const ExecutionReport near_report =
            execution_engine.sendOrder(
                near_order
            );

        const ExecutionReport far_report =
            execution_engine.sendOrder(
                far_order
            );

        /*
         * Update risk state from actual executions.
         */
        near_risk_manager.onExecution(
            near_report
        );

        far_risk_manager.onExecution(
            far_report
        );

        const bool near_filled =
            near_report.status ==
            OrderStatus::FILLED;

        const bool far_filled =
            far_report.status ==
            OrderStatus::FILLED;

        if (near_filled) {
            ++executed_orders;
        }

        if (far_filled) {
            ++executed_orders;
        }

        if (near_filled && far_filled) {
            ++executed_trades;
        }
    }

    const auto end =
        std::chrono::steady_clock::now();

    const auto elapsed =
        std::chrono::duration_cast<
            std::chrono::nanoseconds>(
                end - start
            ).count();

    const std::size_t market_data_events =
        iterations * 2;

    std::cout << "Trading Engine Benchmark\n";
    std::cout << "========================\n";

    std::cout
        << "Market-data events : "
        << market_data_events
        << '\n';

    std::cout
        << "Generated trades   : "
        << generated_trades
        << '\n';

    std::cout
        << "Generated orders   : "
        << generated_orders
        << '\n';

    std::cout
        << "Executed orders    : "
        << executed_orders
        << '\n';

    std::cout
        << "Executed trades    : "
        << executed_trades
        << '\n';

    std::cout
        << "Total time (ns)    : "
        << elapsed
        << '\n';

    if (elapsed > 0) {

        const double seconds =
            static_cast<double>(elapsed) /
            1'000'000'000.0;

        std::cout
            << "Events/sec         : "
            << static_cast<double>(
                   market_data_events
               ) / seconds
            << '\n';

        std::cout
            << "Orders/sec         : "
            << static_cast<double>(
                   executed_orders
               ) / seconds
            << '\n';

        std::cout
            << "Average ns/event   : "
            << static_cast<double>(elapsed) /
                   static_cast<double>(
                       market_data_events
                   )
            << '\n';
    }

    std::cout << '\n';

    std::cout << "Strategy Latency\n";
    std::cout << "----------------\n";

    std::cout
        << "Samples            : "
        << latency_tracker.count()
        << '\n';

    std::cout
        << "Min (ns)           : "
        << latency_tracker.minLatency()
        << '\n';

    std::cout
        << "Average (ns)       : "
        << latency_tracker.averageLatency()
        << '\n';

    std::cout
        << "P50 (ns)           : "
        << latency_tracker.percentile(0.50)
        << '\n';

    std::cout
        << "P95 (ns)           : "
        << latency_tracker.percentile(0.95)
        << '\n';

    std::cout
        << "P99 (ns)           : "
        << latency_tracker.percentile(0.99)
        << '\n';

    std::cout
        << "P99.9 (ns)         : "
        << latency_tracker.percentile(0.999)
        << '\n';

    std::cout
        << "Max (ns)           : "
        << latency_tracker.maxLatency()
        << '\n';

    return 0;
}