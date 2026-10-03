#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include "../common/Types.hpp"
#include "MarketData.hpp"

#include <cstddef>
#include <functional>
#include <map>

namespace trading {

class OrderBook {
public:
    explicit OrderBook(InstrumentId instrument_id);

    void update(const MarketDataEvent& event);

    InstrumentId instrumentId() const;

    PriceTick bestBid() const;
    PriceTick bestAsk() const;

    Quantity bestBidQuantity() const;
    Quantity bestAskQuantity() const;

    std::size_t bidLevels() const;
    std::size_t askLevels() const;

private:
    using BidBook =
        std::map<
            PriceTick,
            Quantity,
            std::greater<PriceTick>
        >;

    using AskBook =
        std::map<
            PriceTick,
            Quantity
        >;

    InstrumentId instrument_id_;

    BidBook bids_;
    AskBook asks_;
};

} // namespace trading

#endif