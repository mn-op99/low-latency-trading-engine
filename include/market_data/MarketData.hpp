#ifndef MARKET_DATA_HPP
#define MARKET_DATA_HPP

#include "../common/TimeUtils.hpp"
#include "../common/Types.hpp"

#include <cstdint>

namespace trading {

/*
 * A single event received from the market-data feed.
 *
 * The structure is intentionally small because it is
 * copied through the SPSC queue on the hot path.
 */
struct MarketDataEvent {
    InstrumentId instrument_id;

    MarketDataType type;
    Side side;

    PriceTick price;
    Quantity quantity;

    /*
     * Timestamp associated with the original market-data
     * event. For historical replay this comes from the
     * input data.
     */
    std::uint64_t exchange_timestamp;

    /*
     * Local timestamp assigned when the event enters
     * our processing pipeline.
     */
    TimePoint received_at;
};

} // namespace trading

#endif