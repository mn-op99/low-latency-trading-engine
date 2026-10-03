cmake_minimum_required(VERSION 3.16)

project(LowLatencyTradingEngine
        VERSION 1.0
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

find_package(Threads REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# ============================================================
# Main Trading Engine
# ============================================================

add_executable(trading_engine

    src/main.cpp

    src/engine/TradingEngine.cpp

    src/market_data/MarketDataGenerator.cpp
    src/market_data/MarketDataParser.cpp
    src/market_data/OrderBook.cpp

    src/strategy/SpreadStrategy.cpp

    src/risk/RiskManager.cpp

    src/execution/ExecutionEngine.cpp
    src/execution/ExchangeSimulator.cpp

    src/metrics/LatencyTracker.cpp
    src/metrics/BenchmarkStats.cpp
)

target_link_libraries(
    trading_engine
    PRIVATE
        Threads::Threads
)


# ============================================================
# Tests
# ============================================================

add_executable(
    test_order_book

    tests/test_order_book.cpp

    src/market_data/OrderBook.cpp
)

target_link_libraries(
    test_order_book
    PRIVATE
        Threads::Threads
)


add_executable(
    test_spsc_queue

    tests/test_spsc_queue.cpp
)

target_link_libraries(
    test_spsc_queue
    PRIVATE
        Threads::Threads
)


add_executable(
    test_risk_manager

    tests/test_risk_manager.cpp

    src/risk/RiskManager.cpp
)

target_link_libraries(
    test_risk_manager
    PRIVATE
        Threads::Threads
)


add_executable(
    test_exchange

    tests/test_exchange.cpp

    src/execution/ExchangeSimulator.cpp
)

target_link_libraries(
    test_exchange
    PRIVATE
        Threads::Threads
)


add_executable(
    test_strategy

    tests/test_strategy.cpp

    src/market_data/OrderBook.cpp
    src/strategy/SpreadStrategy.cpp
)

target_link_libraries(
    test_strategy
    PRIVATE
        Threads::Threads
)


add_executable(
    test_trading_engine

    tests/test_trading_engine.cpp

    src/engine/TradingEngine.cpp

    src/market_data/MarketDataGenerator.cpp
    src/market_data/MarketDataParser.cpp
    src/market_data/OrderBook.cpp

    src/strategy/SpreadStrategy.cpp

    src/risk/RiskManager.cpp

    src/execution/ExecutionEngine.cpp
    src/execution/ExchangeSimulator.cpp

    src/metrics/LatencyTracker.cpp
)

target_link_libraries(
    test_trading_engine
    PRIVATE
        Threads::Threads
)


# ============================================================
# Benchmarks
# ============================================================

add_executable(
    queue_benchmark

    benchmarks/queue_benchmark.cpp
)

target_link_libraries(
    queue_benchmark
    PRIVATE
        Threads::Threads
)


add_executable(
    order_book_benchmark

    benchmarks/order_book_benchmark.cpp

    src/market_data/OrderBook.cpp
)

target_link_libraries(
    order_book_benchmark
    PRIVATE
        Threads::Threads
)


add_executable(
    strategy_benchmark

    benchmarks/strategy_benchmark.cpp

    src/market_data/MarketDataGenerator.cpp
    src/market_data/MarketDataParser.cpp
    src/market_data/OrderBook.cpp
    src/strategy/SpreadStrategy.cpp
)

target_link_libraries(
    strategy_benchmark
    PRIVATE
        Threads::Threads
)


add_executable(
    engine_benchmark

    benchmarks/engine_benchmark.cpp

    src/market_data/MarketDataGenerator.cpp
    src/market_data/MarketDataParser.cpp
    src/market_data/OrderBook.cpp

    src/strategy/SpreadStrategy.cpp

    src/risk/RiskManager.cpp

    src/execution/ExecutionEngine.cpp
    src/execution/ExchangeSimulator.cpp

    src/metrics/LatencyTracker.cpp
)

target_link_libraries(
    engine_benchmark
    PRIVATE
        Threads::Threads
)