#ifndef THREAD_POOL_COMMON_HPP
#define THREAD_POOL_COMMON_HPP

#include <exception>


namespace thread_pool
{
	enum class QueueOpStatus
	{
		success = 0,
		closed,
		empty,
		full
	};

	class QueueException: public std::exception {};

	class QueueClosedException: public QueueException {};

}

#endif //THREAD_POOL_COMMON_HPP
