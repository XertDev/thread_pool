#ifndef THREAD_POOL__NAIVE_BLOCKING_QUEUE_HPP
#define THREAD_POOL__NAIVE_BLOCKING_QUEUE_HPP

#include "common.hpp"

#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>


namespace thread_pool
{
	template<typename T>
	class NaiveBlockingQueue
	{
	public:
		using value_type = T;

		NaiveBlockingQueue();

		NaiveBlockingQueue(const NaiveBlockingQueue&) = delete;
		NaiveBlockingQueue& operator=(const NaiveBlockingQueue&) = delete;

		void push(const value_type& elem);
		void push(value_type&& elem);

		QueueOpStatus try_push(const value_type& elem);
		QueueOpStatus try_push(value_type&& elem);

		[[nodiscard]] QueueOpStatus wait_push(const value_type& elem);
		[[nodiscard]] QueueOpStatus wait_push(value_type&& elem);

		[[nodiscard]] value_type value_pop();

		[[nodiscard]] QueueOpStatus try_pop(value_type& dest);

		[[nodiscard]] QueueOpStatus wait_pop(value_type& dest);

		void close() noexcept;
		[[nodiscard]] bool closed() const noexcept;

		[[nodiscard]] bool empty() const noexcept;

		[[nodiscard]] std::size_t size() const noexcept;

	private:
		bool closed_;
		mutable std::mutex queue_mutex_;
		std::condition_variable consumers_cv_;
		std::queue<T> queue_;
	};

	template<typename T>
	NaiveBlockingQueue<T>::NaiveBlockingQueue()
			:
			closed_(false)
	{}

	template<typename T>
	void NaiveBlockingQueue<T>::push(const value_type& elem)
	{
		if(wait_push(elem) == QueueOpStatus::closed)
		{
			throw QueueClosedException();
		}
	}

	template<typename T>
	void NaiveBlockingQueue<T>::push(value_type&& elem)
	{
		if(wait_push(std::move(elem)) == QueueOpStatus::closed)
		{
			throw QueueClosedException();
		}
	}

	template<typename T>
	QueueOpStatus NaiveBlockingQueue<T>::try_push(const value_type& elem)
	{
		return wait_push(elem);
	}

	template<typename T>
	QueueOpStatus NaiveBlockingQueue<T>::try_push(value_type&& elem)
	{
		return wait_push(std::move(elem));
	}

	template<typename T>
	QueueOpStatus NaiveBlockingQueue<T>::wait_push(const value_type& elem)
	{
		{
			std::unique_lock queue_lock(queue_mutex_);

			if(closed_) {
				return QueueOpStatus::closed;
			}

			queue_.push(elem);
		}
		consumers_cv_.notify_one();

		return QueueOpStatus::success;
	}

	template<typename T>
	QueueOpStatus NaiveBlockingQueue<T>::wait_push(value_type&& elem)
	{
		{
			std::unique_lock queue_lock(queue_mutex_);

			if(closed_)
			{
				return QueueOpStatus::closed;
			}

			queue_.push(std::move(elem));
		}
		consumers_cv_.notify_one();

		return QueueOpStatus::success;
	}

	template<typename T>
	typename NaiveBlockingQueue<T>::value_type NaiveBlockingQueue<T>::value_pop()
	{
		value_type elem;
		if(wait_pop(elem) == QueueOpStatus::closed)
		{
			throw QueueClosedException();
		}

		return elem;
	}

	template<typename T>
	QueueOpStatus NaiveBlockingQueue<T>::try_pop(value_type& dest)
	{
		{
			std::unique_lock queue_lock(queue_mutex_);
			if(queue_.empty())
			{
				if(closed_)
				{
					return QueueOpStatus::closed;
				}
				else
				{
					return QueueOpStatus::empty;
				}
			}

			dest = std::move(queue_.front());
			queue_.pop();
		}
		return QueueOpStatus::success;
	}

	template<typename T>
	QueueOpStatus NaiveBlockingQueue<T>::wait_pop(value_type& dest)
	{
		{
			std::unique_lock queue_lock(queue_mutex_);

			consumers_cv_.wait(
				queue_lock,
				[&]()
				{
					return !queue_.empty() or closed_;
				}

			);

			if(queue_.empty())
			{
				return QueueOpStatus::closed;
			}
			else
			{
				dest = std::move(queue_.front());
				queue_.pop();
			}
		}
		return QueueOpStatus::success;
	}

	template<typename T>
	void NaiveBlockingQueue<T>::close() noexcept
	{
		{
			std::unique_lock queue_lock(queue_mutex_);
			closed_ = true;
		}
		consumers_cv_.notify_all();
	}

	template<typename T>
	bool NaiveBlockingQueue<T>::closed() const noexcept
	{
		std::unique_lock queue_lock(queue_mutex_);
		return closed_;
	}

	template<typename T>
	bool NaiveBlockingQueue<T>::empty() const noexcept
	{
		std::unique_lock queue_lock(queue_mutex_);
		return queue_.empty();
	}

	template<typename T>
	std::size_t NaiveBlockingQueue<T>::size() const noexcept
	{
		std::unique_lock queue_lock(queue_mutex_);
		return queue_.size();
	}
}

#endif //THREAD_POOL__NAIVE_BLOCKING_QUEUE_HPP
