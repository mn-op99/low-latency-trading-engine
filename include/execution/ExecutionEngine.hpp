#ifndef EXECUTION_ENGINE_HPP
#define EXECUTION_ENGINE_HPP

#include "ExchangeSimulator.hpp"
#include "Order.hpp"

namespace trading {

/*
 * ExecutionEngine provides the interface between the
 * trading engine and the exchange.
 *
 * Keeping this layer separate allows the ExchangeSimulator
 * to later be replaced by another execution implementation
 * without changing the strategy or risk-management code.
 */
class ExecutionEngine {
public:
    explicit ExecutionEngine(
        ExchangeSimulator& exchange
    );

    ExecutionReport sendOrder(
        const Order& order
    );

private:
    ExchangeSimulator& exchange_;
};

} // namespace trading

#endif