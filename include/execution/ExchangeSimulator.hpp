#ifndef EXCHANGE_SIMULATOR_HPP
#define EXCHANGE_SIMULATOR_HPP

#include "../common/Types.hpp"
#include "Order.hpp"

namespace trading {

/*
 * Simple exchange model used for local testing.
 *
 * It does not connect to a real exchange. Instead, it
 * deterministically generates an execution report for
 * every valid order.
 */
class ExchangeSimulator {
public:
    explicit ExchangeSimulator(
        PriceTick execution_slippage = 0
    );

    ExecutionReport execute(
        const Order& order
    );

    PriceTick executionSlippage() const;

private:
    PriceTick execution_slippage_;
};

} // namespace trading

#endif