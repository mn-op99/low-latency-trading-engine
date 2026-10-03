#include "../../include/market_data/MarketDataGenerator.hpp"

namespace trading {

MarketDataGenerator::MarketDataGenerator(
    InstrumentId instrument_id)
    : instrument_id_(instrument_id),
      current_price_(10000),
      event_count_(0),
      current_event_(0)
{
}

MarketDataEvent MarketDataGenerator::nextEvent()
{
    /*
     * Replay events loaded from the market-data file.
     */
    if (current_event_ < events_.size()) {

        MarketDataEvent event =
            events_[current_event_++];

        /*
         * This timestamp represents when the event
         * entered our local processing pipeline.
         */
        event.received_at = now();

        return event;
    }

    /*
     * Generate synthetic data when the generator is
     * used without a loaded replay file.
     *
     * This is useful for benchmarks because the benchmark
     * does not need to repeatedly read from disk.
     */
    MarketDataEvent event{};

    event.instrument_id = instrument_id_;

    /*
     * Synthetic exchange timestamp.
     */
    event.exchange_timestamp = event_count_;

    event.received_at = now();

    /*
     * Small deterministic price movement.
     */
    if (event_count_ % 3 == 0) {
        ++current_price_;
    }
    else if (event_count_ % 3 == 1) {
        --current_price_;
    }

    event.type = MarketDataType::ADD;

    if (event_count_ % 2 == 0) {
        event.side = Side::BUY;
    }
    else {
        event.side = Side::SELL;
    }

    event.price = current_price_;

    event.quantity =
        static_cast<Quantity>(
            10 + (event_count_ % 5)
        );

    ++event_count_;

    return event;
}

bool MarketDataGenerator::loadFromFile(
    const std::string& filename)
{
    if (!parser_.load(filename, events_)) {
        return false;
    }

    current_event_ = 0;

    /*
     * Keep the generator state clean when switching
     * from synthetic generation to replay mode.
     */
    event_count_ = 0;

    return true;
}

bool MarketDataGenerator::hasNext() const
{
    return current_event_ < events_.size();
}

void MarketDataGenerator::reset()
{
    current_event_ = 0;
}

std::size_t MarketDataGenerator::eventCount() const
{
    return events_.size();
}

} // namespace trading