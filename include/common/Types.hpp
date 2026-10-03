#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>

namespace trading {

using InstrumentId = std::uint32_t;
using OrderId = std::uint64_t;

using Quantity = std::uint32_t;

/*
 * Prices are stored as integer ticks instead of
 * floating point values.
 *
 * Example:
 *   100.25 with a tick size of 0.01
 *   is stored as 10025.
 */
using PriceTick = std::int64_t;

enum class Side : std::uint8_t {
    BUY,
    SELL
};

enum class OrderType : std::uint8_t {
    LIMIT,
    MARKET
};

enum class MarketDataType : std::uint8_t {
    ADD,
    CANCEL,
    TRADE
};

enum class OrderStatus : std::uint8_t {
    NEW,
    ACCEPTED,
    PARTIALLY_FILLED,
    FILLED,
    REJECTED,
    CANCELLED
};

} // namespace trading

#endif