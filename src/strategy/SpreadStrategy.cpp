#include "../../include/strategy/SpreadStrategy.hpp"

namespace trading {

SpreadStrategy::SpreadStrategy(
    InstrumentId near_instrument,
    InstrumentId far_instrument,
    PriceTick spread_threshold,
    Quantity order_quantity)
    : near_instrument_(near_instrument),
      far_instrument_(far_instrument),
      next_order_id_(1),
      spread_threshold_(spread_threshold),
      order_quantity_(order_quantity)
{
}

bool SpreadStrategy::onMarketUpdate(
    const OrderBook& near_book,
    const OrderBook& far_book,
    Order& near_order,
    Order& far_order)
{
    /*
     * We buy the near instrument at its best ask.
     */
    const PriceTick near_ask =
        near_book.bestAsk();

    /*
     * We sell the far instrument at its best bid.
     */
    const PriceTick far_bid =
        far_book.bestBid();

    /*
     * A valid spread requires both sides of the
     * two-leg trade to be available.
     */
    if (near_ask == 0 || far_bid == 0) {
        return false;
    }

    /*
     * Positive spread:
     *
     *     far_bid - near_ask
     *
     * If this exceeds our configured threshold,
     * the strategy generates a paired trade.
     */
    const PriceTick spread =
        far_bid - near_ask;

    if (spread < spread_threshold_) {
        return false;
    }

    /*
     * Do not generate an order larger than the
     * available top-of-book liquidity.
     */
    const Quantity near_available =
        near_book.bestAskQuantity();

    const Quantity far_available =
        far_book.bestBidQuantity();

    const Quantity executable_quantity =
        near_available < far_available
            ? near_available
            : far_available;

    if (executable_quantity == 0) {
        return false;
    }

    const Quantity quantity =
        executable_quantity < order_quantity_
            ? executable_quantity
            : order_quantity_;

    if (quantity == 0) {
        return false;
    }

    /*
     * Allocate two unique order IDs.
     */
    const OrderId near_order_id =
        next_order_id_++;

    const OrderId far_order_id =
        next_order_id_++;

    /*
     * -------------------------------------------------
     * First leg: BUY near instrument
     * -------------------------------------------------
     */
    near_order.order_id = near_order_id;
    near_order.instrument_id = near_instrument_;
    near_order.side = Side::BUY;
    near_order.type = OrderType::LIMIT;
    near_order.price = near_ask;
    near_order.quantity = quantity;
    near_order.filled_quantity = 0;
    near_order.status = OrderStatus::NEW;
    near_order.created_at = now();

    /*
     * -------------------------------------------------
     * Second leg: SELL far instrument
     * -------------------------------------------------
     */
    far_order.order_id = far_order_id;
    far_order.instrument_id = far_instrument_;
    far_order.side = Side::SELL;
    far_order.type = OrderType::LIMIT;
    far_order.price = far_bid;
    far_order.quantity = quantity;
    far_order.filled_quantity = 0;
    far_order.status = OrderStatus::NEW;
    far_order.created_at = now();

    return true;
}

PriceTick SpreadStrategy::spreadThreshold() const
{
    return spread_threshold_;
}

Quantity SpreadStrategy::orderQuantity() const
{
    return order_quantity_;
}

} // namespace trading