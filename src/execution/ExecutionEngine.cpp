#include "../../include/execution/ExecutionEngine.hpp"

namespace trading {

ExecutionEngine::ExecutionEngine(
    ExchangeSimulator& exchange)
    : exchange_(exchange)
{
}

ExecutionReport ExecutionEngine::sendOrder(
    const Order& order)
{
    /*
     * The execution engine forwards the order to the
     * configured exchange implementation.
     */
    return exchange_.execute(order);
}

} // namespace trading