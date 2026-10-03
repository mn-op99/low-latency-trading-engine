#include "../../include/market_data/OrderBook.hpp"

namespace trading {

OrderBook::OrderBook(InstrumentId instrument_id)
    : instrument_id_(instrument_id)
{
}

void OrderBook::update(const MarketDataEvent& event)
{
    /*
     * The same order book can only process events
     * belonging to its instrument.
     */
    if (event.instrument_id != instrument_id_) {
        return;
    }

    if (event.type == MarketDataType::ADD) {

        if (event.side == Side::BUY) {
            bids_[event.price] += event.quantity;
        }
        else {
            asks_[event.price] += event.quantity;
        }

        return;
    }

    if (event.type == MarketDataType::CANCEL) {

        if (event.side == Side::BUY) {

            auto it = bids_.find(event.price);

            if (it == bids_.end()) {
                return;
            }

            if (it->second <= event.quantity) {
                bids_.erase(it);
            }
            else {
                it->second -= event.quantity;
            }
        }
        else {

            auto it = asks_.find(event.price);

            if (it == asks_.end()) {
                return;
            }

            if (it->second <= event.quantity) {
                asks_.erase(it);
            }
            else {
                it->second -= event.quantity;
            }
        }

        return;
    }

    if (event.type == MarketDataType::TRADE) {

        /*
         * A BUY trade consumes liquidity from the ask side.
         *
         * A SELL trade consumes liquidity from the bid side.
         */
        if (event.side == Side::BUY) {

            auto it = asks_.find(event.price);

            if (it == asks_.end()) {
                return;
            }

            if (it->second <= event.quantity) {
                asks_.erase(it);
            }
            else {
                it->second -= event.quantity;
            }
        }
        else {

            auto it = bids_.find(event.price);

            if (it == bids_.end()) {
                return;
            }

            if (it->second <= event.quantity) {
                bids_.erase(it);
            }
            else {
                it->second -= event.quantity;
            }
        }
    }
}

InstrumentId OrderBook::instrumentId() const
{
    return instrument_id_;
}

PriceTick OrderBook::bestBid() const
{
    if (bids_.empty()) {
        return 0;
    }

    return bids_.begin()->first;
}

PriceTick OrderBook::bestAsk() const
{
    if (asks_.empty()) {
        return 0;
    }

    return asks_.begin()->first;
}

Quantity OrderBook::bestBidQuantity() const
{
    if (bids_.empty()) {
        return 0;
    }

    return bids_.begin()->second;
}

Quantity OrderBook::bestAskQuantity() const
{
    if (asks_.empty()) {
        return 0;
    }

    return asks_.begin()->second;
}

std::size_t OrderBook::bidLevels() const
{
    return bids_.size();
}

std::size_t OrderBook::askLevels() const
{
    return asks_.size();
}

} // namespace trading