#ifndef SPSC_QUEUE_HPP
#define SPSC_QUEUE_HPP

#include <array>
#include <atomic>
#include <cstddef>
#include <type_traits>

namespace trading {

/*
 * Single Producer / Single Consumer bounded ring buffer.
 *
 * Only one thread may call push().
 * Only one thread may call pop().
 *
 * The queue owns all storage up front, so normal push/pop
 * operations do not allocate memory.
 */
template <typename T, std::size_t Capacity>
class SPSCQueue {
    static_assert(
        Capacity >= 2,
        "SPSCQueue capacity must be at least 2"
    );

    static_assert(
        std::is_copy_assignable<T>::value,
        "SPSCQueue requires a copy-assignable type"
    );

public:
    SPSCQueue() = default;

    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;

    bool push(const T& item)
    {
        const std::size_t head =
            head_.load(std::memory_order_relaxed);

        const std::size_t next_head =
            increment(head);

        /*
         * If advancing head reaches tail, the queue
         * is full.
         */
        if (next_head ==
            tail_.load(std::memory_order_acquire)) {
            return false;
        }

        buffer_[head] = item;

        /*
         * Publish the newly written element only after
         * the data has been copied into the buffer.
         */
        head_.store(
            next_head,
            std::memory_order_release
        );

        return true;
    }

    bool pop(T& item)
    {
        const std::size_t tail =
            tail_.load(std::memory_order_relaxed);

        /*
         * No element is available when consumer and
         * producer positions are equal.
         */
        if (tail ==
            head_.load(std::memory_order_acquire)) {
            return false;
        }

        item = buffer_[tail];

        const std::size_t next_tail =
            increment(tail);

        /*
         * Release the slot so that the producer can
         * reuse it.
         */
        tail_.store(
            next_tail,
            std::memory_order_release
        );

        return true;
    }

    bool empty() const
    {
        return tail_.load(
                   std::memory_order_acquire
               ) ==
               head_.load(
                   std::memory_order_acquire
               );
    }

    bool full() const
    {
        const std::size_t head =
            head_.load(std::memory_order_acquire);

        const std::size_t next_head =
            increment(head);

        return next_head ==
               tail_.load(
                   std::memory_order_acquire
               );
    }

    std::size_t capacity() const
    {
        /*
         * One slot is deliberately left unused to
         * distinguish full from empty.
         */
        return Capacity - 1;
    }

private:
    static constexpr std::size_t increment(
        std::size_t index)
    {
        return (index + 1) % Capacity;
    }

private:
    std::array<T, Capacity> buffer_{};

    /*
     * Keeping producer and consumer positions on
     * separate cache lines reduces false sharing.
     */
    alignas(64)
    std::atomic<std::size_t> head_{0};

    alignas(64)
    std::atomic<std::size_t> tail_{0};
};

} // namespace trading

#endif