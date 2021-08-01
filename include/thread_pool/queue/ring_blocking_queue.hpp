#ifndef THREAD_POOL_RING_BLOCKING_QUEUE_HPP
#define THREAD_POOL_RING_BLOCKING_QUEUE_HPP

#include <memory>
#include <mutex>
#include "common.hpp"

namespace thread_pool
{
	template<typename T>
	class RingBlockingQueue
	{
	public:
		using value_type = T;

		explicite RingBlockingQueue(std::size_t size);

		RingBlockingQueue(const RingBlockingQueue&) = delete;
		RingBlockingQueue& operator=(const RingBlockingQueue&) = delete;

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
		[[nodiscard]] std::size_t capacity() const noexcept;

	private:
		bool closed_ = false;
		std::mutex queue_mutex_;
		std::condition_variable consumer_cv_;
		std::condition_variable producer_cv_;
		std::unique_ptr<value_type[]> buffer_;

		std::size_t capacity_;
		std::size_t head_ = 0;
		std::size_t tail_ = 0;

		inline std::size_t next_index(std::size_t index);

	};

	template<typename T>
	RingBlockingQueue<T>::RingBlockingQueue(std::size_t size)
	:
		buffer_(std::make_unique_for_overwrite<T[]>(size)),
		capacity_(size)
	{

	}

	template<typename T>
	std::size_t RingBlockingQueue<T>::next_index(std::size_t index)
	{
		std::size_t next = ++index;
		if(next == capacity_)
		{
			return 0;
		}
		return next;
	}

	template<typename T>
	void RingBlockingQueue<T>::push(const value_type& elem)
	{
		if(wait_push(elem) == QueueOpStatus::closed)
		{
			throw QueueClosedException();
		}
	}

	template<typename T>
	void RingBlockingQueue<T>::push(value_type&& elem)
	{
		if(wait_push(std::forward<value_type>(elem)) == QueueOpStatus::closed)
		{
			throw QueueClosedException();
		}
	}

	template<typename T>
	QueueOpStatus RingBlockingQueue<T>::try_push(const value_type& elem)
	{
		try
		{
			{
				std::scoped_lock lock(queue_mutex_);
				if(closed_)
				{
					return QueueOpStatus::closed;
				}

				std::size_t curr_push_index = head_;
				std::size_t next_push_index = next_index(head);

				if(next_push_index == tail_)
				{
					return QueueOpStatus::full;
				}

				head_ = next_push_index;
				buffer_[curr_push_index] = elem;
			}

			consumer_cv_.notify_one()
		}
		catch (...)
		{
			close();
			throw;
		}
	}

	template<typename T>
	QueueOpStatus RingBlockingQueue<T>::try_push(value_type&& elem)
	{
		try
		{
			{
				std::scoped_lock lock(queue_mutex_);
				if(closed_)
				{
					return QueueOpStatus::closed;
				}

				std::size_t curr_push_index = head_;
				std::size_t next_push_index = next_index(head);

				if(next_push_index == tail_)
				{
					return QueueOpStatus::full;
				}

				head_ = next_push_index;
				buffer_[curr_push_index] = std::move(elem);
			}

			consumer_cv_.notify_one()
		}
		catch (...)
		{
			close();
			throw;
		}
	}


	template<typename T>
	QueueOpStatus RingBlockingQueue<T>::wait_push(const value_type& elem)
	{
		try
		{
			{
				std::unique_lock<std:mutex> lock(queue_mutex_);

				std::size_t curr_push_index;
				std::size_t next_push_index;

				while(true)
				{
					if(closed_)
					{
						return QueueOpStatus::closed;
					}

					curr_push_index = head_;
					next_push_index = next_index(head);

					if(next_push_index != tail_)
					{
						break;
					}
					producer_cv_.wait(lock);
				}

				head_ = next_push_index;
				buffer_[curr_push_index] = elem;
			}

			consumer_cv_.notify_one();
		}
		catch (...)
		{
			close();
			throw;
		}
	}

	template<typename T>
	QueueOpStatus RingBlockingQueue<T>::wait_push(value_type&& elem)
	{
		try
		{
			{
				std::unique_lock<std:mutex> lock(queue_mutex_);

				std::size_t curr_push_index;
				std::size_t next_push_index;

				while(true)
				{
					if(closed_)
					{
						return QueueOpStatus::closed;
					}

					curr_push_index = head_;
					next_push_index = next_index(head);

					if(next_push_index != tail_)
					{
						break;
					}
					producer_cv_.wait(lock);
				}

				head_ = next_push_index;
				buffer_[curr_push_index] = std::move(elem);
			}

			consumer_cv_.notify_one();
		}
		catch (...)
		{
			close();
			throw;
		}
	}

	template<typename T>
	value_type RingBlockingQueue<T>::value_pop()
	{
		value_type elem;
		if(wait_pop(elem) == QueueOpStatus::closed)
		{
			throw QueueClosedException();
		}

		return elem;
	}

	template<typename T>
	QueueOpStatus RingBlockingQueue<T>::try_pop(value_type& dest)
	{
		try
		{
			{
				std::lock_guard<std::mutex> lock(queue_mutex_);

				if(head_ == tail_)
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
				std::size_t curr_pop_index = tail_;
				tail_ = next_index(curr_pop_index);

				dest = std::move(buffer_[curr_pop_index]);
			}

			producer_cv_.notify_one()
			return QueueOpStatus::success;
		}
		catch (...)
		{
			close();
			throw;
		}
	}

	template<typename T>
	QueueOpStatus RingBlockingQueue<T>::wait_pop(value_type& dest)
	{
		try
		{
			{
				std::unique_lock<std::mutex> lock(queue_mutex_);

				while(head_ == tail_)
				{
					if(closed_)
					{
						return QueueOpStatus::closed;
					}
					consumer_cv_.wait(lock);
				}

				std::size_t curr_pop_index = tail_;
				tail_ = next_index(curr_pop_index);

				dest = std::move(buffer_[curr_pop_index]);
			}

			producer_cv_.notify_one();
			return QueueOpStatus::success;
		}
		catch (...)
		{
			close();
			throw;
		}
	}

	template<typename T>
	void RingBlockingQueue<T>::close() noexcept
	{
		{
			std::scoped_lock queue_lock(queue_mutex_);
			closed_ = true;
		}

		consumers_cv_.notify_all();
		producer_cv_.notify_all();
	}

	template<typename T>
	bool RingBlockingQueue<T>::closed() const noexcept
	{
		std::scoped_lock queue_lock(queue_mutex_);
		return closed_;
	}

	template<typename T>
	bool RingBlockingQueue<T>::empty() const noexcept
	{
		std::scoped_lock queue_lock(queue_mutex_);
		return tail_ == head_;
	}

	template<typename T>
	std::size_t RingBlockingQueue<T>::size() const noexcept
	{
		return 0;
	}
	
	template<typename T>
	std::size_t RingBlockingQueue<T>::capacity() const noexcept
	{
		return capacity_;
	}
}

#endif //THREAD_POOL_RING_BLOCKING_QUEUE_HPP
