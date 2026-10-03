#include "../../include/market_data/MarketDataParser.hpp"

#include <fstream>
#include <sstream>

namespace trading {

bool MarketDataParser::load(
    const std::string& filename,
    std::vector<MarketDataEvent>& events) const
{
    std::ifstream file(filename);

    if (!file.is_open()) {
        return false;
    }

    events.clear();

    std::string line;

    // Skip the CSV header.
    std::getline(file, line);

    while (std::getline(file, line)) {

        if (line.empty()) {
            continue;
        }

        std::stringstream stream(line);
        std::string field;

        MarketDataEvent event{};

        // Timestamp
        if (!std::getline(stream, field, ',')) {
            continue;
        }

        event.exchange_timestamp =
            static_cast<std::uint64_t>(
                std::stoull(field)
            );

        // Instrument ID
        if (!std::getline(stream, field, ',')) {
            continue;
        }

        event.instrument_id =
            static_cast<InstrumentId>(
                std::stoul(field)
            );

        // Market-data type
        if (!std::getline(stream, field, ',')) {
            continue;
        }

        if (field == "ADD") {
            event.type = MarketDataType::ADD;
        }
        else if (field == "CANCEL") {
            event.type = MarketDataType::CANCEL;
        }
        else if (field == "TRADE") {
            event.type = MarketDataType::TRADE;
        }
        else {
            continue;
        }

        // Side
        if (!std::getline(stream, field, ',')) {
            continue;
        }

        if (field == "BUY") {
            event.side = Side::BUY;
        }
        else if (field == "SELL") {
            event.side = Side::SELL;
        }
        else {
            continue;
        }

        // Price
        if (!std::getline(stream, field, ',')) {
            continue;
        }

        event.price = std::stoll(field);

        // Quantity
        if (!std::getline(stream, field, ',')) {
            continue;
        }

        event.quantity =
            static_cast<Quantity>(
                std::stoul(field)
            );

        /*
         * This is assigned when the event actually enters
         * the processing pipeline, not when the historical
         * CSV file was parsed.
         */
        event.received_at = TimePoint{};

        events.push_back(event);
    }

    return !events.empty();
}

} // namespace trading