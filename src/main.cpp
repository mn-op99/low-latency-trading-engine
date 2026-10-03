#include "../include/engine/TradingEngine.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    using namespace trading;

    constexpr InstrumentId near_instrument = 1;
    constexpr InstrumentId far_instrument = 2;

    TradingEngine engine(
        near_instrument,
        far_instrument
    );

    /*
     * If a CSV file is supplied, replay that market-data
     * stream. Otherwise, use the synthetic generator.
     */
    if (argc > 1) {

        const std::string filename = argv[1];

        if (!engine.loadMarketData(filename)) {
            std::cerr
                << "Failed to load market data: "
                << filename
                << '\n';

            return 1;
        }

        std::cout
            << "Loaded market data from: "
            << filename
            << '\n';
    }
    else {
        std::cout
            << "No market-data file supplied.\n"
            << "Using synthetic market-data replay.\n";
    }

    /*
     * Run the producer/consumer trading pipeline.
     */
    engine.run();

    /*
     * -----------------------------------------------------
     * Results
     * -----------------------------------------------------
     */
    std::cout << '\n';
    std::cout << "Trading Engine Results\n";
    std::cout << "=====================\n";

    std::cout
        << "Processed events : "
        << engine.processedEvents()
        << '\n';

    std::cout
        << "Generated orders : "
        << engine.generatedOrders()
        << '\n';

    std::cout
        << "Executed orders  : "
        << engine.executedOrders()
        << '\n';

    std::cout
        << "Generated trades : "
        << engine.generatedTrades()
        << '\n';

    std::cout
        << "Executed trades  : "
        << engine.executedTrades()
        << '\n';

    std::cout
        << "Near position    : "
        << engine.nearPosition()
        << '\n';

    std::cout
        << "Far position     : "
        << engine.farPosition()
        << '\n';

    /*
     * -----------------------------------------------------
     * Latency statistics
     * -----------------------------------------------------
     */
    const LatencyTracker& latency =
        engine.latencyTracker();

    std::cout << '\n';
    std::cout << "Latency Statistics\n";
    std::cout << "==================\n";

    std::cout
        << "Samples          : "
        << latency.count()
        << '\n';

    std::cout
        << "Min latency (ns) : "
        << latency.minLatency()
        << '\n';

    std::cout
        << "Avg latency (ns) : "
        << latency.averageLatency()
        << '\n';

    std::cout
        << "P50 latency (ns) : "
        << latency.percentile(0.50)
        << '\n';

    std::cout
        << "P95 latency (ns) : "
        << latency.percentile(0.95)
        << '\n';

    std::cout
        << "P99 latency (ns) : "
        << latency.percentile(0.99)
        << '\n';

    std::cout
        << "P99.9 latency(ns): "
        << latency.percentile(0.999)
        << '\n';

    std::cout
        << "Max latency (ns) : "
        << latency.maxLatency()
        << '\n';

    return 0;
}