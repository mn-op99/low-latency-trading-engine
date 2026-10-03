#include "../include/common/Types.hpp"
#include "../include/market_data/MarketData.hpp"
#include "../include/market_data/OrderBook.hpp"

#include <chrono>
#include <cstddef>
#include <iostream>

int main()
{
    using namespace trading;

    constexpr InstrumentId instrument_id = 1;
    constexpr std::size_t iterations = 1'000'000;

    OrderBook order_book(instrument_id);

    std::size_t successful_updates = 0;

    const auto start =
        std::chrono::steady_clock::now();

    for (std::size_t i = 0; i < iterations; ++i) {

        MarketDataEvent event{};

        event.instrument_id = instrument_id;
        event.type = MarketDataType::ADD;

        /*
         * Alternate between BUY and SELL updates so that
         * both sides of the book are exercised.
         */
        if (i % 2 == 0) {
            event.side = Side::BUY;
        }
        else {
            event.side = Side::SELL;
        }

        /*
         * Keep the number of price levels bounded so that
         * the benchmark measures repeated order-book updates
         * rather than continuously growing the map.
         */
        event.price =
            10000 +
            static_cast<PriceTick>(i % 100);

        event.quantity = 1;

        order_book.update(event);

        ++successful_updates;
    }

    const auto end =
        std::chrono::steady_clock::now();

    const auto elapsed =
        std::chrono::duration_cast<
            std::chrono::nanoseconds>(
                end - start
            ).count();

    std::cout << "Order Book Benchmark\n";
    std::cout << "--------------------\n";

    std::cout
        << "Updates          : "
        << successful_updates
        << '\n';

    std::cout
        << "Time (ns)        : "
        << elapsed
        << '\n';

    if (successful_updates > 0) {

        std::cout
            << "Average ns/update: "
            << static_cast<double>(elapsed) /
                   static_cast<double>(
                       successful_updates
                   )
            << '\n';
    }

    std::cout
        << "Bid levels       : "
        << order_book.bidLevels()
        << '\n';

    std::cout
        << "Ask levels       : "
        << order_book.askLevels()
        << '\n';

    std::cout
        << "Best bid         : "
        << order_book.bestBid()
        << '\n';

    std::cout
        << "Best ask         : "
        << order_book.bestAsk()
        << '\n';

    return 0;
}