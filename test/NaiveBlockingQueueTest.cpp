#include "thread_pool/queue/naive_blocking_queue.hpp"
#include "CommonQueueTest.hpp"

#include <gtest/gtest.h>

using namespace thread_pool;

template <>
NaiveBlockingQueue<int> createQueue(size_t)
{
	return NaiveBlockingQueue<int>();
}

using NaiveBlockingQueueImplementation = testing::Types<NaiveBlockingQueue<int>>;
INSTANTIATE_TYPED_TEST_SUITE_P(
	NaiveBlockingQueueCommonTest,
	CommonQueueTest,
	NaiveBlockingQueueImplementation,
);