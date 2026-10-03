#include "../include/engine/TradingEngine.hpp"

#include <cassert>
#include <iostream>

using namespace trading;

int main()
{
    constexpr InstrumentId near_instrument = 1;
    constexpr InstrumentId far_instrument = 2;

    TradingEngine engine(
        near_instrument,
        far_instrument
    );

    /*
     * Use the synthetic market-data generator.
     *
     * The engine should be able to run without a CSV
     * replay file.
     */
    engine.run();

    /*
     * With no loaded replay file, MarketDataGenerator::hasNext()
     * is initially false, so the engine should terminate cleanly.
     */
    assert(engine.processedEvents() == 0);
    assert(engine.generatedOrders() == 0);
    assert(engine.executedOrders() == 0);
    assert(engine.generatedTrades() == 0);
    assert(engine.executedTrades() == 0);

    /*
     * No market-data events means no latency samples.
     */
    assert(engine.latencyTracker().count() == 0);

    /*
     * No executions should have changed either position.
     */
    assert(engine.nearPosition() == 0);
    assert(engine.farPosition() == 0);

    std::cout << "Trading engine tests passed\n";

    return 0;
}