#include "../../include/risk/RiskManager.hpp"

namespace trading {

RiskManager::RiskManager(
    Quantity max_order_quantity,
    Quantity max_position)
    : max_order_quantity_(max_order_quantity),
      max_position_(max_position),
      position_(0)
{
}

bool RiskManager::validate(const Order& order) const
{
    /*
     * Reject zero-sized orders.
     */
    if (order.quantity == 0) {
        return false;
    }

    /*
     * Reject orders exceeding the per-order limit.
     */
    if (order.quantity > max_order_quantity_) {
        return false;
    }

    /*
     * Calculate the position that would exist if
     * the complete order were executed.
     */
    const std::int64_t order_quantity =
        static_cast<std::int64_t>(
            order.quantity
        );

    std::int64_t projected_position =
        position_;

    if (order.side == Side::BUY) {
        projected_position += order_quantity;
    }
    else {
        projected_position -= order_quantity;
    }

    const std::int64_t position_limit =
        static_cast<std::int64_t>(
            max_position_
        );

    /*
     * The resulting position must remain within
     * [-max_position, +max_position].
     */
    if (projected_position > position_limit) {
        return false;
    }

    if (projected_position < -position_limit) {
        return false;
    }

    return true;
}

void RiskManager::onExecution(
    const ExecutionReport& report)
{
    /*
     * Rejected/cancelled orders do not change
     * the position.
     */
    if (report.status != OrderStatus::FILLED &&
        report.status != OrderStatus::PARTIALLY_FILLED) {
        return;
    }

    const std::int64_t quantity =
        static_cast<std::int64_t>(
            report.execution_quantity
        );

    if (report.side == Side::BUY) {
        position_ += quantity;
    }
    else {
        position_ -= quantity;
    }
}

std::int64_t RiskManager::currentPosition() const
{
    return position_;
}

Quantity RiskManager::maxOrderQuantity() const
{
    return max_order_quantity_;
}

Quantity RiskManager::maxPosition() const
{
    return max_position_;
}

} // namespace trading