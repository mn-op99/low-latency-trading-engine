#include "../include/queue/SPSCQueue.hpp"

#include <cassert>
#include <iostream>

using namespace trading;

int main()
{
    SPSCQueue<int, 4> queue;

    assert(queue.empty());

    /*
     * Capacity is 4, so the ring buffer can
     * hold 3 elements at the same time.
     */
    assert(queue.push(10));
    assert(queue.push(20));
    assert(queue.push(30));

    /*
     * Queue is full here.
     */
    assert(!queue.push(40));

    int value = 0;

    assert(queue.pop(value));
    assert(value == 10);

    assert(queue.pop(value));
    assert(value == 20);

    /*
     * A free slot is available again.
     */
    assert(queue.push(40));

    assert(queue.pop(value));
    assert(value == 30);

    assert(queue.pop(value));
    assert(value == 40);

    assert(queue.empty());

    /*
     * Popping from an empty queue should fail.
     */
    assert(!queue.pop(value));

    std::cout << "SPSC queue tests passed\n";

    return 0;
}