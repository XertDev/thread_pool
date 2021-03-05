cmake_minimum_required(VERSION 3.17)
project(thread_pool CXX)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
	add_compile_options(/W4 /WX)
else()
	add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

enable_testing()
find_package(GTest)

add_library(thread_pool INTERFACE)
target_include_directories(thread_pool INTERFACE include/)
target_sources(
		thread_pool INTERFACE
		include/thread_pool/thread_pool.hpp
		include/thread_pool/queue/naive_blocking_queue.hpp
		include/thread_pool/queue/common.hpp
		include/thread_pool/detail/_queue_requirement.hpp
		)

add_executable(thread_pool_test "")
target_link_libraries(thread_pool_test GTest::GTest gtest_main thread_pool)
add_subdirectory(test)

add_test(test thread_pool_test)