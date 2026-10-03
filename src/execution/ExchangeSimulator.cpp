#include "../../include/execution/ExchangeSimulator.hpp"

namespace trading {

ExchangeSimulator::ExchangeSimulator(
    PriceTick execution_slippage)
    : execution_slippage_(execution_slippage)
{
}

ExecutionReport ExchangeSimulator::execute(
    const Order& order)
{
    ExecutionReport report{};

    report.order_id = order.order_id;
    report.instrument_id = order.instrument_id;
    report.side = order.side;
    report.execution_quantity = 0;
    report.execution_price = order.price;
    report.status = OrderStatus::REJECTED;
    report.timestamp = now();

    /*
     * An order with zero quantity is invalid.
     */
    if (order.quantity == 0) {
        return report;
    }

    /*
     * In this simulator every valid order is completely
     * filled.
     *
     * Slippage is applied against the trader:
     *
     * BUY  -> pay a higher price
     * SELL -> receive a lower price
     */
    if (order.side == Side::BUY) {
        report.execution_price =
            order.price + execution_slippage_;
    }
    else {
        report.execution_price =
            order.price - execution_slippage_;
    }

    report.execution_quantity = order.quantity;
    report.status = OrderStatus::FILLED;

    return report;
}

PriceTick ExchangeSimulator::executionSlippage() const
{
    return execution_slippage_;
}

} // namespace trading