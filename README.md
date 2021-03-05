## Thread pool
<span>Simple header-only library for processing tasks in thread pool written in c++20.</span>
<br>
<a target="_blank" href="LICENSE" title="License: MIT"><img src="https://img.shields.io/badge/License-MIT-blue.svg"></a>

### Example
```c++

	thread_pool::ThreadPool thread_pool;

	auto task_1 = thread_pool.enqueue([](){ return 7; });
	auto task_2 = thread_pool.enqueue([](){ return std::string("test"); });

	std::cout << task_1.get() << "\n">; //7
	std::cout << task_2.get() << "\n">; //test
```

### License
MIT © Xert