#include "../include/strategy/SpreadStrategy.hpp"

#include <cassert>
#include <iostream>

using namespace trading;

int main()
{
    constexpr InstrumentId near_instrument = 1;
    constexpr InstrumentId far_instrument = 2;

    /*
     * Generate a signal when:
     *
     *     far_bid - near_ask >= 5
     *
     * and request a maximum of 10 units.
     */
    SpreadStrategy strategy(
        near_instrument,
        far_instrument,
        5,
        10
    );

    OrderBook near_book(near_instrument);
    OrderBook far_book(far_instrument);

    /*
     * Near instrument:
     *
     * Best ask = 10000
     * Available quantity = 20
     */
    MarketDataEvent near_ask{};

    near_ask.instrument_id = near_instrument;
    near_ask.type = MarketDataType::ADD;
    near_ask.side = Side::SELL;
    near_ask.price = 10000;
    near_ask.quantity = 20;

    near_book.update(near_ask);

    /*
     * Far instrument:
     *
     * Best bid = 10010
     * Available quantity = 15
     *
     * Spread = 10010 - 10000 = 10
     */
    MarketDataEvent far_bid{};

    far_bid.instrument_id = far_instrument;
    far_bid.type = MarketDataType::ADD;
    far_bid.side = Side::BUY;
    far_bid.price = 10010;
    far_bid.quantity = 15;

    far_book.update(far_bid);

    Order near_order{};
    Order far_order{};

    const bool signal_generated =
        strategy.onMarketUpdate(
            near_book,
            far_book,
            near_order,
            far_order
        );

    assert(signal_generated);

    /*
     * Quantity is limited by:
     *
     *   strategy quantity = 10
     *   near liquidity     = 20
     *   far liquidity      = 15
     *
     * Therefore quantity = 10.
     */
    assert(near_order.quantity == 10);
    assert(far_order.quantity == 10);

    assert(near_order.instrument_id == near_instrument);
    assert(far_order.instrument_id == far_instrument);

    assert(near_order.side == Side::BUY);
    assert(far_order.side == Side::SELL);

    assert(near_order.price == 10000);
    assert(far_order.price == 10010);

    assert(near_order.type == OrderType::LIMIT);
    assert(far_order.type == OrderType::LIMIT);

    assert(near_order.status == OrderStatus::NEW);
    assert(far_order.status == OrderStatus::NEW);

    assert(near_order.order_id != far_order.order_id);

    /*
     * Verify that a spread below the threshold does
     * not generate a trade.
     */
    OrderBook small_spread_near(near_instrument);
    OrderBook small_spread_far(far_instrument);

    MarketDataEvent near_ask_small{};

    near_ask_small.instrument_id = near_instrument;
    near_ask_small.type = MarketDataType::ADD;
    near_ask_small.side = Side::SELL;
    near_ask_small.price = 10000;
    near_ask_small.quantity = 20;

    small_spread_near.update(near_ask_small);

    MarketDataEvent far_bid_small{};

    far_bid_small.instrument_id = far_instrument;
    far_bid_small.type = MarketDataType::ADD;
    far_bid_small.side = Side::BUY;
    far_bid_small.price = 10004;
    far_bid_small.quantity = 20;

    small_spread_far.update(far_bid_small);

    Order no_trade_near{};
    Order no_trade_far{};

    const bool no_signal =
        strategy.onMarketUpdate(
            small_spread_near,
            small_spread_far,
            no_trade_near,
            no_trade_far
        );

    assert(!no_signal);

    /*
     * Verify that insufficient liquidity limits the
     * generated order quantity.
     */
    OrderBook limited_near(near_instrument);
    OrderBook limited_far(far_instrument);

    MarketDataEvent limited_near_event{};

    limited_near_event.instrument_id = near_instrument;
    limited_near_event.type = MarketDataType::ADD;
    limited_near_event.side = Side::SELL;
    limited_near_event.price = 10000;
    limited_near_event.quantity = 4;

    limited_near.update(limited_near_event);

    MarketDataEvent limited_far_event{};

    limited_far_event.instrument_id = far_instrument;
    limited_far_event.type = MarketDataType::ADD;
    limited_far_event.side = Side::BUY;
    limited_far_event.price = 10010;
    limited_far_event.quantity = 20;

    limited_far.update(limited_far_event);

    Order limited_near_order{};
    Order limited_far_order{};

    const bool limited_signal =
        strategy.onMarketUpdate(
            limited_near,
            limited_far,
            limited_near_order,
            limited_far_order
        );

    assert(limited_signal);

    /*
     * Near side only has 4 units available, so both
     * legs must be limited to 4 units.
     */
    assert(limited_near_order.quantity == 4);
    assert(limited_far_order.quantity == 4);

    /*
     * Verify that a missing best bid/ask prevents
     * signal generation.
     */
    OrderBook empty_near(near_instrument);
    OrderBook empty_far(far_instrument);

    Order empty_near_order{};
    Order empty_far_order{};

    const bool empty_signal =
        strategy.onMarketUpdate(
            empty_near,
            empty_far,
            empty_near_order,
            empty_far_order
        );

    assert(!empty_signal);

    std::cout << "Spread strategy tests passed\n";

    return 0;
}