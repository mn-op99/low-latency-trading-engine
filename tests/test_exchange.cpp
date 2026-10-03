#include "../include/execution/ExchangeSimulator.hpp"

#include <cassert>
#include <iostream>

using namespace trading;

int main()
{
    ExchangeSimulator exchange;

    Order buy_order{};

    buy_order.order_id = 1;
    buy_order.instrument_id = 1;
    buy_order.side = Side::BUY;
    buy_order.type = OrderType::LIMIT;
    buy_order.price = 10000;
    buy_order.quantity = 50;

    ExecutionReport buy_report =
        exchange.execute(buy_order);

    assert(
        buy_report.status ==
        OrderStatus::FILLED
    );

    assert(
        buy_report.execution_quantity == 50
    );

    assert(
        buy_report.execution_price == 10000
    );

    /*
     * Check SELL execution.
     */
    Order sell_order{};

    sell_order.order_id = 2;
    sell_order.instrument_id = 1;
    sell_order.side = Side::SELL;
    sell_order.type = OrderType::LIMIT;
    sell_order.price = 10020;
    sell_order.quantity = 25;

    ExecutionReport sell_report =
        exchange.execute(sell_order);

    assert(
        sell_report.status ==
        OrderStatus::FILLED
    );

    assert(
        sell_report.execution_quantity == 25
    );

    assert(
        sell_report.execution_price == 10020
    );

    /*
     * Zero quantity must be rejected.
     */
    Order invalid_order{};

    invalid_order.order_id = 3;
    invalid_order.instrument_id = 1;
    invalid_order.side = Side::BUY;
    invalid_order.type = OrderType::LIMIT;
    invalid_order.price = 10000;
    invalid_order.quantity = 0;

    ExecutionReport invalid_report =
        exchange.execute(invalid_order);

    assert(
        invalid_report.status ==
        OrderStatus::REJECTED
    );

    assert(
        invalid_report.execution_quantity == 0
    );

    /*
     * Check simulated execution slippage.
     */
    ExchangeSimulator slippage_exchange(2);

    ExecutionReport slippage_buy =
        slippage_exchange.execute(
            buy_order
        );

    assert(
        slippage_buy.execution_price == 10002
    );

    ExecutionReport slippage_sell =
        slippage_exchange.execute(
            sell_order
        );

    assert(
        slippage_sell.execution_price == 10018
    );

    std::cout << "Exchange simulator tests passed\n";

    return 0;
}