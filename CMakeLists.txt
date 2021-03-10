cmake_minimum_required(VERSION 3.17)
project(thread_pool CXX)

set(CMAKE_CXX_STANDARD 20)

enable_testing()
find_package(GTest QUIET)

if (NOT GTest_FOUND)
	include(FetchContent)
	FetchContent_Declare(
			googletest
			GIT_REPOSITORY https://github.com/google/googletest.git
			GIT_TAG        release-1.10.0
	)

	FetchContent_GetProperties(googletest)
	if(NOT googletest_POPULATED)
		FetchContent_Populate(googletest)
		set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE BOOL "")
		add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
		unset(CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
		add_library(GTest::gtest ALIAS gtest)
		add_library(GTest::gtest_main ALIAS gtest_main)
	endif()
endif()

add_library(thread_pool INTERFACE)
target_include_directories(thread_pool INTERFACE include/)
target_sources(
		thread_pool INTERFACE
		include/thread_pool/thread_pool.hpp
		include/thread_pool/queue/naive_blocking_queue.hpp
		include/thread_pool/queue/common.hpp
		include/thread_pool/detail/_task.hpp
		include/thread_pool/detail/_queue_requirement.hpp
		)

add_executable(thread_pool_test "")
target_link_libraries(thread_pool_test GTest::gtest GTest::gtest_main thread_pool)
add_subdirectory(test)

add_test(test thread_pool_test)

if (MSVC)
	target_compile_options(thread_pool_test PRIVATE /W4 /WX)
else()
	target_compile_options(thread_pool_test PRIVATE -Wall -Wextra -pedantic -Werror)
endif()