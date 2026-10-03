#include "../include/risk/RiskManager.hpp"

#include <cassert>
#include <iostream>

using namespace trading;

int main()
{
    RiskManager risk_manager(100, 500);

    Order order{};

    order.order_id = 1;
    order.instrument_id = 1;
    order.side = Side::BUY;
    order.type = OrderType::LIMIT;
    order.price = 10000;
    order.quantity = 50;

    /*
     * Normal order should pass.
     */
    assert(risk_manager.validate(order));

    /*
     * Order quantity limit.
     */
    order.quantity = 150;

    assert(!risk_manager.validate(order));

    /*
     * Zero quantity should be rejected.
     */
    order.quantity = 0;

    assert(!risk_manager.validate(order));

    /*
     * Restore a valid order.
     */
    order.quantity = 50;

    assert(risk_manager.validate(order));

    /*
     * Simulate a filled BUY order.
     */
    ExecutionReport buy_report{};

    buy_report.order_id = order.order_id;
    buy_report.instrument_id = order.instrument_id;
    buy_report.side = Side::BUY;
    buy_report.execution_quantity = 50;
    buy_report.status = OrderStatus::FILLED;

    risk_manager.onExecution(buy_report);

    assert(risk_manager.currentPosition() == 50);

    /*
     * A position that stays within the limit
     * should still allow another BUY.
     */
    assert(risk_manager.validate(order));

    /*
     * Simulate a SELL execution.
     */
    ExecutionReport sell_report{};

    sell_report.order_id = 2;
    sell_report.instrument_id = 1;
    sell_report.side = Side::SELL;
    sell_report.execution_quantity = 50;
    sell_report.status = OrderStatus::FILLED;

    risk_manager.onExecution(sell_report);

    assert(risk_manager.currentPosition() == 0);

    /*
     * Rejected executions must not change position.
     */
    ExecutionReport rejected_report{};

    rejected_report.order_id = 3;
    rejected_report.instrument_id = 1;
    rejected_report.side = Side::BUY;
    rejected_report.execution_quantity = 100;
    rejected_report.status = OrderStatus::REJECTED;

    risk_manager.onExecution(rejected_report);

    assert(risk_manager.currentPosition() == 0);

    std::cout << "Risk manager tests passed\n";

    return 0;
}