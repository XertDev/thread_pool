#ifndef THREAD_POOL_UTILS_HPP
#define THREAD_POOL_UTILS_HPP

#include "thread_pool/detail/_queue_requirement.hpp"


template <typename Q>
requires thread_pool::detail::task_queue<Q>
Q createQueue(size_t size);

#endif //THREAD_POOL_UTILS_HPP
