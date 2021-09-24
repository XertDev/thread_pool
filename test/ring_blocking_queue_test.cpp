#include "thread_pool/queue/ring_blocking_queue.hpp"
#include "common_queue_test.hpp"
#include "sized_queue_test.hpp"


using namespace thread_pool;

using QueueType = RingBlockingQueue<int>;

template <>
QueueType createQueue(size_t size)
{
	return QueueType(size);
}

using RingBlockingQueueImplementation = testing::Types<QueueType>;

INSTANTIATE_TYPED_TEST_SUITE_P(
	RingBlockingQueueCommonTest,
	common_queue_test,
	RingBlockingQueueImplementation,
);

INSTANTIATE_TYPED_TEST_SUITE_P(
	RingBlockingQueueSizedTest,
	sized_queue_test,
	RingBlockingQueueImplementation,
);