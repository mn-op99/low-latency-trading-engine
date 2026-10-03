#include "../../include/engine/TradingEngine.hpp"

#include <thread>

namespace trading {

TradingEngine::TradingEngine(
    InstrumentId near_instrument,
    InstrumentId far_instrument)
    : near_instrument_(near_instrument),
      far_instrument_(far_instrument),
      market_data_(near_instrument),
      market_data_queue_(),
      near_order_book_(near_instrument),
      far_order_book_(far_instrument),
      strategy_(
          near_instrument,
          far_instrument,
          5,
          10
      ),
      near_risk_manager_(100, 1000),
      far_risk_manager_(100, 1000),
      exchange_(),
      execution_engine_(exchange_),
      latency_tracker_(100000),
      producer_done_(false),
      processed_events_(0),
      generated_orders_(0),
      executed_orders_(0),
      generated_trades_(0),
      executed_trades_(0)
{
}

TradingEngine::~TradingEngine()
{
    /*
     * Normally run() joins both threads before returning.
     *
     * This guard makes destruction safe even if the engine
     * is destroyed after a future change introduces another
     * exit path.
     */
    if (producer_thread_.joinable()) {
        producer_thread_.join();
    }

    if (consumer_thread_.joinable()) {
        consumer_thread_.join();
    }
}

bool TradingEngine::loadMarketData(
    const std::string& filename)
{
    return market_data_.loadFromFile(filename);
}

void TradingEngine::run()
{
    /*
     * run() is intended to process one complete market-data
     * replay. Reset the producer completion flag before
     * starting the threads.
     */
    producer_done_.store(
        false,
        std::memory_order_relaxed
    );

    producer_thread_ =
        std::thread(
            &TradingEngine::marketDataLoop,
            this
        );

    consumer_thread_ =
        std::thread(
            &TradingEngine::processingLoop,
            this
        );

    producer_thread_.join();
    consumer_thread_.join();
}

void TradingEngine::marketDataLoop()
{
    while (market_data_.hasNext()) {

        MarketDataEvent event =
            market_data_.nextEvent();

        /*
         * If the queue is temporarily full, wait until the
         * consumer frees a slot.
         *
         * No mutex is required because this is an SPSC queue.
         */
        while (!market_data_queue_.push(event)) {
            std::this_thread::yield();
        }
    }

    /*
     * Release ordering ensures that all queue writes made
     * by the producer happen before the consumer observes
     * producer_done_ == true.
     */
    producer_done_.store(
        true,
        std::memory_order_release
    );
}

void TradingEngine::processingLoop()
{
    MarketDataEvent event{};

    /*
     * Continue until:
     *
     *   1. producer has finished, AND
     *   2. the queue has been completely drained.
     */
    while (!producer_done_.load(
               std::memory_order_acquire
           ) ||
           !market_data_queue_.empty()) {

        /*
         * Nothing is currently available. Yield rather than
         * continuously executing the consumer loop.
         */
        if (!market_data_queue_.pop(event)) {
            std::this_thread::yield();
            continue;
        }

        processed_events_.fetch_add(
            1,
            std::memory_order_relaxed
        );

        /*
         * received_at was assigned by the producer when
         * the market-data event entered the local pipeline.
         *
         * Measuring from this point captures queue waiting
         * time plus downstream processing time.
         */
        const TimePoint processing_start =
            event.received_at;

        /*
         * Route the event to the corresponding order book.
         */
        if (event.instrument_id == near_instrument_) {

            near_order_book_.update(event);
        }
        else if (event.instrument_id == far_instrument_) {

            far_order_book_.update(event);
        }
        else {
            /*
             * Ignore instruments that are not part of this
             * strategy.
             */
            continue;
        }

        /*
         * Orders are constructed locally. This avoids
         * dynamic allocation in the processing path.
         */
        Order near_order{};
        Order far_order{};

        const bool signal_generated =
            strategy_.onMarketUpdate(
                near_order_book_,
                far_order_book_,
                near_order,
                far_order
            );

        /*
         * Measure market-data-to-strategy processing latency
         * regardless of whether a trade signal was generated.
         */
        const TimePoint processing_end =
            now();

        latency_tracker_.record(
            processing_start,
            processing_end
        );

        if (!signal_generated) {
            continue;
        }

        generated_trades_.fetch_add(
            1,
            std::memory_order_relaxed
        );

        /*
         * Every spread signal contains two legs.
         */
        generated_orders_.fetch_add(
            2,
            std::memory_order_relaxed
        );

        /*
         * Each leg has an independent risk check because
         * the instruments have independent position limits.
         */
        if (!near_risk_manager_.validate(near_order)) {
            continue;
        }

        if (!far_risk_manager_.validate(far_order)) {
            continue;
        }

        /*
         * Execute the first leg.
         */
        const ExecutionReport near_report =
            execution_engine_.sendOrder(
                near_order
            );

        /*
         * Execute the second leg.
         */
        const ExecutionReport far_report =
            execution_engine_.sendOrder(
                far_order
            );

        /*
         * Position state is updated only from actual
         * execution reports.
         */
        near_risk_manager_.onExecution(
            near_report
        );

        far_risk_manager_.onExecution(
            far_report
        );

        const bool near_filled =
            near_report.status ==
            OrderStatus::FILLED;

        const bool far_filled =
            far_report.status ==
            OrderStatus::FILLED;

        if (near_filled) {
            executed_orders_.fetch_add(
                1,
                std::memory_order_relaxed
            );
        }

        if (far_filled) {
            executed_orders_.fetch_add(
                1,
                std::memory_order_relaxed
            );
        }

        /*
         * A complete spread trade requires both legs
         * to execute successfully.
         */
        if (near_filled && far_filled) {
            executed_trades_.fetch_add(
                1,
                std::memory_order_relaxed
            );
        }
    }
}

std::uint64_t TradingEngine::processedEvents() const
{
    return processed_events_.load(
        std::memory_order_relaxed
    );
}

std::uint64_t TradingEngine::generatedOrders() const
{
    return generated_orders_.load(
        std::memory_order_relaxed
    );
}

std::uint64_t TradingEngine::executedOrders() const
{
    return executed_orders_.load(
        std::memory_order_relaxed
    );
}

std::uint64_t TradingEngine::generatedTrades() const
{
    return generated_trades_.load(
        std::memory_order_relaxed
    );
}

std::uint64_t TradingEngine::executedTrades() const
{
    return executed_trades_.load(
        std::memory_order_relaxed
    );
}

std::int64_t TradingEngine::nearPosition() const
{
    return near_risk_manager_.currentPosition();
}

std::int64_t TradingEngine::farPosition() const
{
    return far_risk_manager_.currentPosition();
}

const LatencyTracker&
TradingEngine::latencyTracker() const
{
    return latency_tracker_;
}

} // namespace trading