#ifndef THREAD_POOL_THREAD_POOL_HPP
#define THREAD_POOL_THREAD_POOL_HPP

#include "detail/_queue_requirement.hpp"
#include "detail/_task.hpp"
#include "queue/naive_blocking_queue.hpp"

#include <thread>
#include <future>
#include <vector>
#include <concepts>
#include <cassert>


namespace thread_pool {
	template<template <typename> class Q = NaiveBlockingQueue>
			requires detail::task_queue<Q<detail::Task>>
	class ThreadPool
	{
	public:
		explicit ThreadPool(std::size_t thread_count=std::thread::hardware_concurrency()) {
			assert(thread_count != 0);

			for(std::size_t i = 0; i < thread_count; ++i)
			{
				workers_.emplace_back
				(
					[&]()
					{
						detail::Task work;
						while(true)
						{
							auto state = tasks_.wait_pop(work);
							if(state == QueueOpStatus::closed)
							{
								return;
							}
							work();
						}
					}
				);
			}
		}

		~ThreadPool()
		{
			tasks_.close();
			for(auto& worker: workers_)
			{
				worker.join();
			}
		}

		template<typename F, typename... Args>
		requires std::invocable<F, Args...>
		auto enqueue(F fun, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
		{
			auto task = std::packaged_task<std::invoke_result_t<F, Args...>()>
					(
							[f = std::move(fun), ...f_args = std::forward<Args>(args)]() mutable
							{
								return f(std::forward<Args>(f_args)...);
							}
					);

			auto task_future = task.get_future();
			tasks_.push([worker_task = std::move(task)]() mutable { worker_task(); });

			return task_future;
		}

		[[nodiscard]] std::size_t thread_count() const noexcept
		{
			return workers_.size();
		}

	private:
		std::vector<std::thread> workers_;

		Q<detail::Task> tasks_;
	};
}

#endif //THREAD_POOL_THREAD_POOL_HPP