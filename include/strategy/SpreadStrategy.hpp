#ifndef SPREAD_STRATEGY_HPP
#define SPREAD_STRATEGY_HPP

#include "../common/Types.hpp"
#include "../execution/Order.hpp"
#include "../market_data/OrderBook.hpp"

namespace trading {

class SpreadStrategy {
public:
    SpreadStrategy(
        InstrumentId near_instrument,
        InstrumentId far_instrument,
        PriceTick spread_threshold,
        Quantity order_quantity
    );

    /*
     * Evaluates the current two-leg spread.
     *
     * Returns true when a trade should be generated.
     * The two resulting orders are written into near_order
     * and far_order.
     */
    bool onMarketUpdate(
        const OrderBook& near_book,
        const OrderBook& far_book,
        Order& near_order,
        Order& far_order
    );

    PriceTick spreadThreshold() const;

    Quantity orderQuantity() const;

private:
    InstrumentId near_instrument_;
    InstrumentId far_instrument_;

    OrderId next_order_id_;

    PriceTick spread_threshold_;
    Quantity order_quantity_;
};

} // namespace trading

#endif