#include "../include/common/Types.hpp"
#include "../include/market_data/MarketDataGenerator.hpp"
#include "../include/market_data/OrderBook.hpp"
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

    std::size_t signals_generated = 0;
    std::size_t orders_generated = 0;

    const auto start =
        std::chrono::steady_clock::now();

    for (std::size_t i = 0; i < iterations; ++i) {

        /*
         * Generate one market-data event for each leg.
         */
        MarketDataEvent near_event =
            near_generator.nextEvent();

        MarketDataEvent far_event =
            far_generator.nextEvent();

        /*
         * Update both order books before evaluating
         * the spread.
         */
        near_book.update(near_event);
        far_book.update(far_event);

        Order near_order{};
        Order far_order{};

        /*
         * Measure the strategy's ability to process the
         * current two-book state and generate a paired
         * order.
         */
        if (strategy.onMarketUpdate(
                near_book,
                far_book,
                near_order,
                far_order)) {

            ++signals_generated;

            orders_generated += 2;
        }
    }

    const auto end =
        std::chrono::steady_clock::now();

    const auto elapsed =
        std::chrono::duration_cast<
            std::chrono::nanoseconds>(
                end - start
            ).count();

    const std::size_t events_processed =
        iterations * 2;

    std::cout << "Strategy Benchmark\n";
    std::cout << "------------------\n";

    std::cout
        << "Market-data events : "
        << events_processed
        << '\n';

    std::cout
        << "Signals generated  : "
        << signals_generated
        << '\n';

    std::cout
        << "Orders generated   : "
        << orders_generated
        << '\n';

    std::cout
        << "Time (ns)          : "
        << elapsed
        << '\n';

    if (elapsed > 0) {

        const double seconds =
            static_cast<double>(elapsed) /
            1'000'000'000.0;

        std::cout
            << "Events/sec         : "
            << static_cast<double>(
                   events_processed
               ) / seconds
            << '\n';

        std::cout
            << "Signals/sec        : "
            << static_cast<double>(
                   signals_generated
               ) / seconds
            << '\n';

        std::cout
            << "Average ns/event   : "
            << static_cast<double>(elapsed) /
                   static_cast<double>(
                       events_processed
                   )
            << '\n';
    }

    return 0;
}