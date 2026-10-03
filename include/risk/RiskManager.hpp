#ifndef RISK_MANAGER_HPP
#define RISK_MANAGER_HPP

#include "../common/Types.hpp"
#include "../execution/Order.hpp"

#include <cstdint>

namespace trading {

class RiskManager {
public:
    RiskManager(
        Quantity max_order_quantity,
        Quantity max_position
    );

    /*
     * Validates an order against:
     *   1. Non-zero quantity
     *   2. Maximum order size
     *   3. Maximum position limit
     */
    bool validate(const Order& order) const;

    /*
     * Updates the current position after an execution.
     */
    void onExecution(const ExecutionReport& report);

    std::int64_t currentPosition() const;

    Quantity maxOrderQuantity() const;

    Quantity maxPosition() const;

private:
    Quantity max_order_quantity_;
    Quantity max_position_;

    /*
     * Positive  -> net long
     * Negative  -> net short
     */
    std::int64_t position_;
};

} // namespace trading

#endif