#ifndef THREAD_POOL_SIZED_QUEUE_TEST_HPP
#define THREAD_POOL_SIZED_QUEUE_TEST_HPP

#include <gtest/gtest.h>

#include "utils.hpp"
#include "thread_pool/detail/_queue_requirement.hpp"
#include "thread_pool/queue/common.hpp"
#include <concepts>
#include <iterator>
#include <thread_pool/queue/naive_blocking_queue.hpp>


template <typename Q>
requires thread_pool::detail::task_queue<Q>
class sized_queue_test : public testing::Test
{
public:
	static constexpr size_t size = 15;
protected:
	sized_queue_test()
	: queue(createQueue<Q>(size))
	{}

	Q queue;
};

TYPED_TEST_SUITE_P(sized_queue_test);

TYPED_TEST_P(sized_queue_test, invalid_initial_size)
{
	using QueueType = decltype(this->queue);
	EXPECT_THROW(QueueType(0), std::invalid_argument);
}

TYPED_TEST_P(sized_queue_test, capacity)
{
	using QueueType = decltype(this->queue);

	{
		const QueueType queue(1);
		EXPECT_EQ(1, queue.capacity());
	}

	{
		const QueueType queue(25);
		EXPECT_EQ(25, queue.capacity());
	}
}

TYPED_TEST_P(sized_queue_test, try_pop_empty)
{
	ASSERT_TRUE(this->queue.empty());
	ASSERT_FALSE(this->queue.closed());

	int val;

	EXPECT_EQ(thread_pool::QueueOpStatus::empty, this->queue.try_pop(val));
	EXPECT_TRUE(this->queue.empty());
	EXPECT_FALSE(this->queue.closed());

	this->queue.push(9);
	EXPECT_FALSE(this->queue.empty());
	EXPECT_FALSE(this->queue.closed());

	EXPECT_EQ(thread_pool::QueueOpStatus::success, this->queue.try_pop(val));
	EXPECT_EQ(9, val);

	EXPECT_TRUE(this->queue.empty());
	EXPECT_FALSE(this->queue.closed());
}

TYPED_TEST_P(sized_queue_test, try_push_full)
{
	ASSERT_TRUE(this->queue.empty());
	ASSERT_FALSE(this->queue.closed());

	for(size_t i = 0; i < this->size; ++i)
	{
		EXPECT_FALSE(this->queue.full());
		this->queue.push(9);
	}
	ASSERT_TRUE(this->queue.full());
	EXPECT_EQ(thread_pool::QueueOpStatus::full, this->queue.try_push(8));
}

REGISTER_TYPED_TEST_SUITE_P(
	sized_queue_test,
	invalid_initial_size,
	capacity,
	try_pop_empty,
	try_push_full
);

#endif //THREAD_POOL_SIZED_QUEUE_TEST_HPP
