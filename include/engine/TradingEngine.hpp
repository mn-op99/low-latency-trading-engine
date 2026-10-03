#ifndef TRADING_ENGINE_HPP
#define TRADING_ENGINE_HPP

#include "../execution/ExecutionEngine.hpp"
#include "../execution/ExchangeSimulator.hpp"
#include "../market_data/MarketDataGenerator.hpp"
#include "../market_data/OrderBook.hpp"
#include "../metrics/LatencyTracker.hpp"
#include "../queue/SPSCQueue.hpp"
#include "../risk/RiskManager.hpp"
#include "../strategy/SpreadStrategy.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>
#include <thread>

namespace trading {

class TradingEngine {
public:
    TradingEngine(
        InstrumentId near_instrument,
        InstrumentId far_instrument
    );

    ~TradingEngine();

    TradingEngine(const TradingEngine&) = delete;
    TradingEngine& operator=(const TradingEngine&) = delete;

    /*
     * Loads historical market-data events from a CSV file.
     */
    bool loadMarketData(
        const std::string& filename
    );

    /*
     * Starts the producer and consumer threads and
     * processes the complete market-data stream.
     */
    void run();

    /*
     * Runtime statistics.
     */
    std::uint64_t processedEvents() const;

    std::uint64_t generatedOrders() const;

    std::uint64_t executedOrders() const;

    std::uint64_t generatedTrades() const;

    std::uint64_t executedTrades() const;

    /*
     * Current positions of the two instruments.
     */
    std::int64_t nearPosition() const;

    std::int64_t farPosition() const;

    /*
     * End-to-end processing latency:
     *
     * market-data timestamp
     *        ->
     * order-book update
     *        ->
     * strategy
     *        ->
     * risk decision
     */
    const LatencyTracker& latencyTracker() const;

private:
    /*
     * Producer thread:
     *
     * MarketDataGenerator -> SPSCQueue
     */
    void marketDataLoop();

    /*
     * Consumer thread:
     *
     * SPSCQueue -> OrderBook -> Strategy
     *            -> Risk -> Execution
     */
    void processingLoop();

private:
    /*
     * The queue is intentionally bounded and preallocated.
     */
    static constexpr std::size_t QueueSize = 4096;

    InstrumentId near_instrument_;
    InstrumentId far_instrument_;

    /*
     * Market-data producer.
     */
    MarketDataGenerator market_data_;

    /*
     * Single producer / single consumer queue.
     */
    SPSCQueue<
        MarketDataEvent,
        QueueSize
    > market_data_queue_;

    /*
     * Independent order books for the two instruments.
     */
    OrderBook near_order_book_;
    OrderBook far_order_book_;

    /*
     * Trading signal generation.
     */
    SpreadStrategy strategy_;

    /*
     * Independent risk limits for each instrument.
     */
    RiskManager near_risk_manager_;
    RiskManager far_risk_manager_;

    /*
     * Execution path.
     */
    ExchangeSimulator exchange_;
    ExecutionEngine execution_engine_;

    /*
     * Processing latency measurements.
     */
    LatencyTracker latency_tracker_;

    /*
     * Signals the consumer that the producer has
     * completely drained the market-data source.
     */
    std::atomic<bool> producer_done_;

    /*
     * Runtime counters.
     */
    std::atomic<std::uint64_t> processed_events_;
    std::atomic<std::uint64_t> generated_orders_;
    std::atomic<std::uint64_t> executed_orders_;

    std::atomic<std::uint64_t> generated_trades_;
    std::atomic<std::uint64_t> executed_trades_;

    /*
     * Producer and consumer threads.
     */
    std::thread producer_thread_;
    std::thread consumer_thread_;
};

} // namespace trading

#endif