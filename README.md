# Low-Latency Market Data & Execution Engine

A student-scale C++ trading engine designed to explore low-latency market-data processing, event-driven execution, multithreading, lock-free communication, and latency benchmarking.

The project models a simplified trading pipeline:

Market Data → SPSC Queue → Order Book → Strategy → Risk → Execution → Exchange Simulator

The implementation is intentionally self-contained and does not connect to a real exchange.

---

## 1. Project Overview

The engine simulates a two-instrument spread-trading system.

Two instruments are treated as:

- Near instrument
- Far instrument

The strategy looks for an opportunity where:

```text
Far Best Bid - Near Best Ask >= Spread Threshold
```

When the condition is satisfied, the engine generates two orders:

```text
BUY  Near Instrument
SELL Far  Instrument
```

Both orders pass through risk validation before being sent to the exchange simulator.

---

## 2. Architecture

```text
                    MARKET DATA
                         │
                         ▼
                ┌─────────────────┐
                │ MarketDataParser│
                └────────┬────────┘
                         │
                         ▼
                ┌─────────────────┐
                │ MarketData      │
                │ Replay/Generator│
                └────────┬────────┘
                         │
                         ▼
                ┌─────────────────┐
                │   SPSC Queue    │
                │   Lock-free     │
                └────────┬────────┘
                         │
                    Consumer
                      Thread
                         │
                         ▼
              ┌──────────────────────┐
              │     Order Books      │
              │                      │
              │ Near       Far       │
              └──────────┬───────────┘
                         │
                         ▼
                ┌─────────────────┐
                │ Spread Strategy │
                └────────┬────────┘
                         │
                  BUY Near +
                  SELL Far
                         │
                         ▼
                ┌─────────────────┐
                │  Risk Manager   │
                └────────┬────────┘
                         │
                         ▼
                ┌─────────────────┐
                │Execution Engine │
                └────────┬────────┘
                         │
                         ▼
                ┌──────────────────┐
                │Exchange Simulator│
                └────────┬─────────┘
                         │
                         ▼
                 Execution Reports
                         │
                         ▼
                  Position / Risk
```

---

## 3. Main Components

### Market Data

`MarketDataGenerator`

Provides:

- Synthetic market-data events
- Historical CSV replay

`MarketDataParser`

Reads events from CSV files.

Supported event types:

```text
ADD
CANCEL
TRADE
```

---

### SPSC Queue

`SPSCQueue`

A bounded Single Producer / Single Consumer ring buffer.

Properties:

- Preallocated storage
- No dynamic allocation during push/pop
- Atomic producer and consumer indices
- Acquire/release memory ordering
- Cache-line alignment to reduce false sharing

The queue is used between the market-data producer thread and processing thread.

---

### Order Book

`OrderBook`

Maintains:

```text
Bid levels
Ask levels
```

The current implementation uses ordered maps.

For bids:

```text
Highest price → Best bid
```

For asks:

```text
Lowest price → Best ask
```

Supported updates:

```text
ADD
CANCEL
TRADE
```

Prices are represented using integer ticks rather than floating-point values.

For example:

```text
100.25
```

with a tick size of:

```text
0.01
```

is represented as:

```text
10025
```

---

## 4. Trading Strategy

`SpreadStrategy`

The strategy uses the top of both order books.

The spread is calculated as:

```text
spread = far_best_bid - near_best_ask
```

A trade signal is generated when:

```text
spread >= spread_threshold
```

The generated pair is:

```text
Near: BUY at best ask

Far:  SELL at best bid
```

The order quantity is constrained by the available liquidity on both sides.

Therefore:

```text
trade_quantity =
    min(
        configured_order_quantity,
        near_available_quantity,
        far_available_quantity
    )
```

---

## 5. Risk Management

`RiskManager`

The risk layer applies basic position controls.

Each instrument has:

```text
Maximum order quantity
Maximum position
```

An order is rejected when:

- Quantity is zero
- Quantity exceeds the maximum order size
- Projected position exceeds the position limit

Position updates happen from execution reports rather than from order submission.

---

## 6. Execution

`ExecutionEngine`

Provides the abstraction between the trading engine and exchange.

Currently the execution target is:

`ExchangeSimulator`

The simulator:

- Accepts valid non-zero orders
- Generates execution reports
- Fully fills accepted orders
- Supports configurable execution slippage

For a BUY:

```text
execution_price =
    order_price + slippage
```

For a SELL:

```text
execution_price =
    order_price - slippage
```

This allows execution behavior to be benchmarked without requiring a live exchange connection.

---

## 7. Latency Measurement

`LatencyTracker`

The engine records processing latency from the time a market-data event enters the local pipeline until the corresponding strategy processing completes.

The project reports:

```text
Minimum
Average
P50
P95
P99
P99.9
Maximum
```

All latency values are reported in nanoseconds.

`std::chrono::steady_clock` is used because it provides a monotonic clock suitable for elapsed-time measurement.

---

## 8. Benchmarking

The project contains separate benchmarks for the major components.

### Queue Benchmark

```text
benchmarks/queue_benchmark.cpp
```

Measures:

- Push/pop throughput
- Average nanoseconds per operation

---

### Order Book Benchmark

```text
benchmarks/order_book_benchmark.cpp
```

Measures:

- Order-book update throughput
- Average nanoseconds per update

---

### Strategy Benchmark

```text
benchmarks/strategy_benchmark.cpp
```

Measures:

- Market-data processing
- Spread-signal generation
- Average processing time

---

### Engine Benchmark

```text
benchmarks/engine_benchmark.cpp
```

Measures the core single-threaded processing pipeline:

```text
Market Data
    ↓
Order Book
    ↓
Strategy
    ↓
Risk
    ↓
Execution
```

It reports:

- Events/sec
- Orders/sec
- Average ns/event
- P50 latency
- P95 latency
- P99 latency
- P99.9 latency
- Maximum latency

---

## 9. Low-Latency Design Techniques

The project focuses on several techniques relevant to low-latency C++ systems.

### Preallocation

The SPSC queue allocates its storage when constructed rather than during event processing.

Latency samples can also be reserved ahead of time.

---

### Lock-Free Communication

The producer and consumer communicate through an SPSC ring buffer.

The queue uses:

```cpp
std::atomic<std::size_t>
```

with acquire/release memory ordering.

No mutex is used for the market-data queue.

---

### Integer Price Representation

Prices use integer ticks:

```cpp
using PriceTick = std::int64_t;
```

This avoids floating-point arithmetic for price comparisons.

---

### Avoiding Hot-Path Allocation

The main event-processing path uses stack-allocated structures such as:

```text
MarketDataEvent
Order
ExecutionReport
```

The SPSC queue itself uses preallocated storage.

---

### Cache-Line Alignment

Producer and consumer indices are aligned to separate cache lines:

```cpp
alignas(64)
```

This is intended to reduce false sharing between the producer and consumer threads.

---

## 10. Project Structure

```text
low_latency_trading_engine/
│
├── CMakeLists.txt
├── README.md
│
├── include/
│   ├── common/
│   │   ├── Types.hpp
│   │   └── TimeUtils.hpp
│   │
│   ├── market_data/
│   │   ├── MarketData.hpp
│   │   ├── MarketDataGenerator.hpp
│   │   ├── MarketDataParser.hpp
│   │   └── OrderBook.hpp
│   │
│   ├── queue/
│   │   └── SPSCQueue.hpp
│   │
│   ├── strategy/
│   │   └── SpreadStrategy.hpp
│   │
│   ├── risk/
│   │   └── RiskManager.hpp
│   │
│   ├── execution/
│   │   ├── Order.hpp
│   │   ├── ExecutionEngine.hpp
│   │   └── ExchangeSimulator.hpp
│   │
│   ├── metrics/
│   │   ├── LatencyTracker.hpp
│   │   └── BenchmarkStats.hpp
│   │
│   └── engine/
│       └── TradingEngine.hpp
│
├── src/
│   ├── main.cpp
│   ├── engine/
│   │   └── TradingEngine.cpp
│   ├── market_data/
│   │   ├── MarketDataGenerator.cpp
│   │   ├── MarketDataParser.cpp
│   │   └── OrderBook.cpp
│   ├── strategy/
│   │   └── SpreadStrategy.cpp
│   ├── risk/
│   │   └── RiskManager.cpp
│   ├── execution/
│   │   ├── ExecutionEngine.cpp
│   │   └── ExchangeSimulator.cpp
│   └── metrics/
│       ├── LatencyTracker.cpp
│       └── BenchmarkStats.cpp
│
├── tests/
│   ├── test_order_book.cpp
│   ├── test_spsc_queue.cpp
│   ├── test_risk_manager.cpp
│   ├── test_exchange.cpp
│   ├── test_strategy.cpp
│   └── test_trading_engine.cpp
│
├── benchmarks/
│   ├── queue_benchmark.cpp
│   ├── order_book_benchmark.cpp
│   ├── strategy_benchmark.cpp
│   └── engine_benchmark.cpp
│
└── data/
    └── sample_market_data.csv
```

---

## 11. Market Data Format

The sample replay file uses:

```csv
timestamp,instrument_id,type,side,price,quantity
```

Example:

```csv
1,1,ADD,BUY,9995,20
2,2,ADD,BUY,10020,20
3,1,ADD,SELL,10005,15
4,2,ADD,SELL,10035,15
```

Prices are stored as integer ticks.

---

## 12. Building

The project uses CMake and requires a C++17-compatible compiler.

Create a build directory:

```bash
mkdir build
cd build
```

Configure:

```bash
cmake ..
```

Build:

```bash
cmake --build . -j
```

---

## 13. Running

Run with the sample market-data file:

```bash
./trading_engine ../data/sample_market_data.csv
```

The engine will:

```text
Read market data
      ↓
Producer thread
      ↓
SPSC queue
      ↓
Consumer thread
      ↓
Order books
      ↓
Spread strategy
      ↓
Risk checks
      ↓
Execution simulator
      ↓
Statistics
```

---

## 14. Running Without a Replay File

The executable can also be started without a CSV argument:

```bash
./trading_engine
```

In that case the market-data generator is used without a loaded replay stream.

---

## 15. Tests

The project contains component-level tests for:

```text
Order Book
SPSC Queue
Risk Manager
Exchange Simulator
Spread Strategy
Trading Engine
```

The tests are built as separate CMake targets.

---

## 16. Performance Analysis

The project is intended to be benchmarked under different workloads.

Useful measurements include:

```text
Events/sec
Orders/sec
Average ns/event
P50 latency
P95 latency
P99 latency
P99.9 latency
Maximum latency
```

For Linux systems, profiling can additionally be performed with tools such as:

```bash
perf stat
perf record
perf report
```

This can help identify:

- CPU hotspots
- Branch-heavy sections
- Cache behavior
- Function-level CPU usage
- Synchronization overhead

---

## 17. Current Scope

This is a research/learning-oriented trading-engine implementation rather than a production HFT system.

The current implementation does not claim to provide:

- Kernel-bypass networking
- DPDK
- RDMA
- FPGA acceleration
- Hardware timestamping
- Exchange co-location
- Production exchange connectivity
- Production-grade order recovery
- Production-grade market-data sequencing
- Nanosecond-level deterministic execution

The focus is on understanding and measuring the software-engineering techniques that contribute to low-latency systems.

---

## 18. Potential Extensions

Possible future improvements include:

```text
Optimized order-book representation
        ↓
Memory-pool based order management
        ↓
More realistic exchange matching
        ↓
Partial fills
        ↓
Order cancellation
        ↓
Two-leg execution recovery
        ↓
More detailed latency instrumentation
        ↓
CPU affinity
        ↓
NUMA-aware processing
        ↓
Linux performance profiling
        ↓
Multi-threaded feed handling
        ↓
Real market-data adapter
```

These extensions can be evaluated experimentally by comparing throughput, latency distributions, and CPU behavior against the existing implementation.

---

## 19. Key Learning Objectives

This project demonstrates practical understanding of:

- Modern C++
- RAII and object ownership
- STL data structures
- Atomics
- Memory ordering
- Lock-free SPSC queues
- Cache-line alignment
- False-sharing considerations
- Multithreaded event processing
- Order-book data structures
- Event-driven architecture
- Risk management
- Execution modeling
- Latency measurement
- Benchmark design
- CMake
- Linux performance profiling

---

## 20. Disclaimer

This project is a simplified educational trading-system implementation.

It is intended to demonstrate systems programming, performance engineering, and trading-engine concepts in a controlled environment. It should not be used for live trading without substantial additional engineering, testing, risk controls, and exchange-specific infrastructure.
