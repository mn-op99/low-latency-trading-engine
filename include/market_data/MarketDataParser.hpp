#ifndef MARKET_DATA_PARSER_HPP
#define MARKET_DATA_PARSER_HPP

#include "MarketData.hpp"

#include <string>
#include <vector>

namespace trading {

class MarketDataParser {
public:
    bool load(
        const std::string& filename,
        std::vector<MarketDataEvent>& events) const;
};

} // namespace trading

#endif