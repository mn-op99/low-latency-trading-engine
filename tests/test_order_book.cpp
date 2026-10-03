#include "../include/market_data/OrderBook.hpp"

#include <cassert>
#include <iostream>

using namespace trading;

int main()
{
    OrderBook book(1);

    MarketDataEvent bid{};

    bid.instrument_id = 1;
    bid.type = MarketDataType::ADD;
    bid.side = Side::BUY;
    bid.price = 10000;
    bid.quantity = 50;

    book.update(bid);

    assert(book.bestBid() == 10000);
    assert(book.bestBidQuantity() == 50);

    MarketDataEvent better_bid = bid;

    better_bid.price = 10005;
    better_bid.quantity = 20;

    book.update(better_bid);

    assert(book.bestBid() == 10005);
    assert(book.bestBidQuantity() == 20);

    MarketDataEvent ask{};

    ask.instrument_id = 1;
    ask.type = MarketDataType::ADD;
    ask.side = Side::SELL;
    ask.price = 10010;
    ask.quantity = 30;

    book.update(ask);

    assert(book.bestAsk() == 10010);
    assert(book.bestAskQuantity() == 30);

    /*
     * Cancel part of the original bid level.
     */
    MarketDataEvent cancel_bid = bid;

    cancel_bid.type = MarketDataType::CANCEL;
    cancel_bid.price = 10000;
    cancel_bid.quantity = 20;

    book.update(cancel_bid);

    assert(book.bidLevels() == 2);

    /*
     * Cancel the remaining quantity at 10000.
     */
    cancel_bid.quantity = 30;

    book.update(cancel_bid);

    assert(book.bidLevels() == 1);
    assert(book.bestBid() == 10005);

    /*
     * A trade on the sell side consumes
     * quantity from the bid book.
     */
    MarketDataEvent trade{};

    trade.instrument_id = 1;
    trade.type = MarketDataType::TRADE;
    trade.side = Side::SELL;
    trade.price = 10005;
    trade.quantity = 5;

    book.update(trade);

    assert(book.bestBid() == 10005);
    assert(book.bestBidQuantity() == 15);

    /*
     * Ignore events belonging to another instrument.
     */
    MarketDataEvent other_instrument = ask;

    other_instrument.instrument_id = 2;
    other_instrument.price = 10100;

    book.update(other_instrument);

    assert(book.bestAsk() == 10010);

    std::cout << "Order book tests passed\n";

    return 0;
}