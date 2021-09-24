#include "thread_pool/queue/naive_blocking_queue.hpp"
#include "common_queue_test.hpp"


using namespace thread_pool;

template <>
NaiveBlockingQueue<int> createQueue(size_t)
{
	return {};
}

using NaiveBlockingQueueImplementation = testing::Types<NaiveBlockingQueue<int>>;

INSTANTIATE_TYPED_TEST_SUITE_P(
	NaiveBlockingQueueCommonTest,
	common_queue_test,
	NaiveBlockingQueueImplementation,
);