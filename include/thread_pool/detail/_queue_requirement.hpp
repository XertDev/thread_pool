#ifndef THREAD_POOL__QUEUE_REQUIREMENT_HPP
#define THREAD_POOL__QUEUE_REQUIREMENT_HPP

#include "thread_pool/queue/common.hpp"

#include <concepts>


namespace thread_pool::detail
{
	template<typename QueueType>
	concept task_queue = requires(
			QueueType a,
			const QueueType b,
			typename QueueType::value_type& value,
			typename QueueType::value_type&& tmp_value
	)
	{
		typename QueueType::value_type;
		requires std::movable<typename QueueType::value_type>;

		a.push(std::move(tmp_value));
		{ a.value_pop() } -> std::same_as<typename QueueType::value_type>;

		{ a.try_push(std::move(tmp_value)) } -> std::same_as<QueueOpStatus>;
		{ a.try_pop(value) } -> std::same_as<QueueOpStatus>;

		{ a.wait_push(std::move(tmp_value)) } -> std::same_as<QueueOpStatus>;
		{ a.wait_pop(value) } -> std::same_as<QueueOpStatus>;

		a.close();
		{ b.closed() } -> std::same_as<bool>;

		{ b.empty() } -> std::same_as<bool>;
	};
}

#endif //THREAD_POOL__QUEUE_REQUIREMENT_HPP
