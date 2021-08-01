#include <gtest/gtest.h>

#include "thread_pool/detail/_queue_requirement.hpp"
#include "thread_pool/queue/common.hpp"
#include <concepts>
#include <iterator>

template <typename Q>
requires thread_pool::detail::task_queue<Q>
Q createQueue(size_t size);

template <typename Q>
requires thread_pool::detail::task_queue<Q>
class CommonQueueTest : public testing::Test
{
public:
	static constexpr size_t size = 15;
protected:
	CommonQueueTest()
	: queue(createQueue<Q>(size))
	{}

	Q queue;
};

template<typename Q, typename C>
requires thread_pool::detail::task_queue<Q>
         && std::input_iterator<typename C::const_iterator>
void push_container(Q& queue, C container)
{
	for(const auto& val: container)
	{
		queue.push(val);
		ASSERT_FALSE(queue.empty());
		ASSERT_FALSE(queue.closed());
	}
}

template<typename Q, typename C>
requires thread_pool::detail::task_queue<Q>
         && std::input_iterator<typename C::const_iterator>
void pop_container(Q& queue, C container)
{
	for(const auto& expected: container)
	{
		ASSERT_FALSE(queue.empty());

		const auto value = queue.value_pop();

		ASSERT_EQ(expected, value);
	}
}

template<typename Q, typename C>
requires thread_pool::detail::task_queue<Q>
         && std::input_iterator<typename C::const_iterator>
void try_push_container(Q& queue, C container)
{
	for(const auto& val: container)
	{
		const auto result = queue.try_push(val);
		ASSERT_EQ(thread_pool::QueueOpStatus::success, result);
		ASSERT_FALSE(queue.empty());
		ASSERT_FALSE(queue.closed());
	}
}

template<typename Q, typename C>
requires thread_pool::detail::task_queue<Q>
         && std::input_iterator<typename C::const_iterator>
void try_pop_container(Q& queue, C container)
{
	for(const auto& expected: container)
	{
		ASSERT_FALSE(queue.empty());

		int val;
		const auto result = queue.try_pop(val);

		ASSERT_EQ(thread_pool::QueueOpStatus::success, result);
		ASSERT_EQ(expected, val);
	}
}

TYPED_TEST_SUITE_P(CommonQueueTest);

TYPED_TEST_P(CommonQueueTest, initial_setup)
{
	EXPECT_TRUE(this->queue.empty());
	EXPECT_FALSE(this->queue.closed());
}

TYPED_TEST_P(CommonQueueTest, closed_after_close)
{
	this->queue.close();

	EXPECT_TRUE(this->queue.closed());
}

TYPED_TEST_P(CommonQueueTest, pop_closed)
{
	this->queue.close();
	EXPECT_THROW(static_cast<void>(this->queue.value_pop()), thread_pool::QueueClosedException);
}

TYPED_TEST_P(CommonQueueTest, push_pop)
{
	this->queue.push(1);

	ASSERT_FALSE(this->queue.empty());
	ASSERT_FALSE(this->queue.closed());

	ASSERT_EQ(1, this->queue.value_pop());
}

TYPED_TEST_P(CommonQueueTest, multiple_push_pop)
{
	const std::array test_array = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

	push_container(this->queue, test_array);
	pop_container(this->queue, test_array);

	ASSERT_TRUE(this->queue.empty());
}

TYPED_TEST_P(CommonQueueTest, try_push_pop)
{
	ASSERT_EQ(thread_pool::QueueOpStatus::success, this->queue.try_push(10));

	ASSERT_FALSE(this->queue.empty());
	ASSERT_FALSE(this->queue.closed());

	int value;

	ASSERT_EQ(thread_pool::QueueOpStatus::success, this->queue.try_pop(value));

	ASSERT_TRUE(this->queue.empty());
	ASSERT_FALSE(this->queue.closed());
}

TYPED_TEST_P(CommonQueueTest, try_pop_empty)
{
	int val;
	ASSERT_TRUE(this->queue.empty());
	ASSERT_EQ(thread_pool::QueueOpStatus::empty, this->queue.try_pop(val));
}

TYPED_TEST_P(CommonQueueTest, try_pop_closed)
{
	this->queue.close();
	ASSERT_TRUE(this->queue.closed());

	int val;
	ASSERT_EQ(thread_pool::QueueOpStatus::closed, this->queue.try_pop(val));
}

TYPED_TEST_P(CommonQueueTest, multiple_try_push_pop)
{
	const std::array test_array = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

	try_push_container(this->queue, test_array);
	try_pop_container(this->queue, test_array);

	ASSERT_TRUE(this->queue.empty());
}

REGISTER_TYPED_TEST_SUITE_P(
	CommonQueueTest,
	initial_setup,
	closed_after_close,
	pop_closed,
	push_pop,
	multiple_push_pop,
	try_push_pop,
	try_pop_empty,
	try_pop_closed,
	multiple_try_push_pop
);