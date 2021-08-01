#include <gtest/gtest.h>
#include <thread_pool/thread_pool.hpp>

TEST(ThreadPoolTest, thread_count)
{
	thread_pool::ThreadPool thread_pool_1;
	thread_pool::ThreadPool thread_pool_2(5);

	ASSERT_EQ(std::thread::hardware_concurrency(), thread_pool_1.thread_count());
	ASSERT_EQ(5, thread_pool_2.thread_count());
}

TEST(ThreadPoolTest, task_enqueue)
{
	thread_pool::ThreadPool thread_pool;

	auto task_1 = thread_pool.enqueue([](){return 7;});
	auto task_2 = thread_pool.enqueue([](){return 8;});

	ASSERT_EQ(7, task_1.get());
	ASSERT_EQ(8, task_2.get());
}

TEST(ThreadPoolTest, task_with_different_results)
{
	thread_pool::ThreadPool thread_pool;

	auto task_1 = thread_pool.enqueue([](){ return 7; });
	auto task_2 = thread_pool.enqueue([](){ return std::string("test"); });

	ASSERT_EQ(7, task_1.get());
	ASSERT_EQ(std::string("test"), task_2.get());
}

TEST(ThreadPoolTest, enqueue_with_arguments)
{
	thread_pool::ThreadPool thread_pool;

	auto task_1 = thread_pool.enqueue([](int x){ return x; }, 1);
	auto task_2 = thread_pool.enqueue([](int x, int z) -> double { return x*z; }, 5, 4);

	ASSERT_EQ(1, task_1.get());
	ASSERT_EQ(20, task_2.get());
}

TEST(ThreadPoolTest, throw_handling)
{
	struct exception_1: public std::exception
	{
		explicit exception_1(int val)
		:
			x(val)
		{}

		int x;
	};
	struct exception_2: public std::exception {};

	thread_pool::ThreadPool thread_pool;

	auto task_1 = thread_pool.enqueue(
			[](int x)
			{
				throw exception_1(x);
			},
			1
		);
	auto task_2 = thread_pool.enqueue([]() { throw exception_2(); });

	try
	{
		task_1.get();
		FAIL() << "exception not thrown";
	}
	catch(const exception_1& e)
	{
		ASSERT_EQ(1, e.x);
	}
	catch(...)
	{
		FAIL() << "invalid exception type";
	}

	ASSERT_THROW(task_2.get(), exception_2);
}

template<template <typename> class T>
class ThreadPoolTest : public testing::Test
{
protected:
	thread_pool::ThreadPool<T> thread_pool_;
};