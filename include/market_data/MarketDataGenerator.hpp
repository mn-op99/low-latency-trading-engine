#ifndef MARKET_DATA_GENERATOR_HPP
#define MARKET_DATA_GENERATOR_HPP

#include "MarketData.hpp"
#include "MarketDataParser.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace trading {

class MarketDataGenerator {
public:
    explicit MarketDataGenerator(
        InstrumentId instrument_id);

    MarketDataEvent nextEvent();

    bool loadFromFile(
        const std::string& filename);

    bool hasNext() const;

    void reset();

    std::size_t eventCount() const;

private:
    InstrumentId instrument_id_;

    PriceTick current_price_;
    std::uint64_t event_count_;

    std::vector<MarketDataEvent> events_;
    std::size_t current_event_;

    MarketDataParser parser_;
};

} // namespace trading

#endif