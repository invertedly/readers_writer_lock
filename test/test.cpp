#include "pch.h"

#include "../readers_writer_lock/reader_lock.h"
#include "../readers_writer_lock/rwlock.h"
#include "../readers_writer_lock/writer_lock.h"

#include <chrono>
#include <future>

class slowpoke_shared_string final
{
	std::string data_;
	uint64_t slowpoke_ms_count_;
	mutable rwlock::rwlock rwlock_;

public:
	slowpoke_shared_string(std::string buf = {}, const uint64_t slowpoke_ms_count = 0)
		: data_(std::move(buf)),
		  slowpoke_ms_count_(slowpoke_ms_count)
	{
	}

	std::string read(const int64_t timeout = -1) const
	{
		rwlock::reader_lock lock(rwlock_, true, timeout);

		std::this_thread::sleep_for(std::chrono::duration<uint64_t, std::milli>(slowpoke_ms_count_));

		return data_;
	}

	void write(const std::string& buf, const int64_t timeout = -1)
	{
		rwlock::writer_lock lock(rwlock_, true, timeout);

		std::this_thread::sleep_for(std::chrono::duration<uint64_t, std::milli>(slowpoke_ms_count_));

		data_ = buf;
	}
};

TEST(readers_writer_lock, lock)
{
	slowpoke_shared_string shared_string{};

	shared_string.write("hello");

	std::vector<std::jthread> threads(5);

	for (auto& thread : threads)
	{
		thread = std::jthread(
			[](const slowpoke_shared_string& shared_string)
			{
				EXPECT_STREQ(shared_string.read().c_str(), "hello");
			},
			std::ref(shared_string)
		);
	}

	std::ranges::for_each(threads, [](std::jthread& jthread) { jthread.join(); });

	shared_string.write("bye");

	EXPECT_STREQ(shared_string.read().c_str(), "bye");
}

void time_test_multi_thread(const size_t thread_count, const uint64_t slowpoke_timeout_ms)
{
	slowpoke_shared_string shared_string{ "initial", slowpoke_timeout_ms };
	std::vector<std::jthread> threads(thread_count);

	std::chrono::steady_clock::time_point time_begin = std::chrono::steady_clock::now();

	for (auto& thread : threads)
	{
		thread = std::jthread(
			&slowpoke_shared_string::read,
			std::ref(shared_string),
			-1
		);
	}

	std::ranges::for_each(threads, [](std::jthread& jthread) { jthread.join(); });

	std::jthread writer(
		&slowpoke_shared_string::write,
		std::ref(shared_string),
		"final",
		-1
	);

	writer.join();

	std::chrono::steady_clock::time_point time_end = std::chrono::steady_clock::now();
	std::cout << "Multi thread \t= " << std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_begin).count() << "[ms]" << std::endl;
}

void time_test_single_thread(const size_t thread_count, const uint64_t slowpoke_timeout_ms)
{
	std::chrono::steady_clock::time_point time_begin = std::chrono::steady_clock::now();

	slowpoke_shared_string shared_string{ "initial", slowpoke_timeout_ms };

	for (int i = 0; i < thread_count; ++i)
	{
		shared_string.read();
	}

	shared_string.write("final");

	std::chrono::steady_clock::time_point time_end = std::chrono::steady_clock::now();
	std::cout << "Single thread \t= " << std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_begin).count() << "[ms]" << std::endl;
}

void time_test(const size_t operation_count, const uint64_t slowpoke_timeout_ms, const size_t test_repeat_count = 1)
{
	std::cout << "Operation count = " << operation_count << "; Operation duration = " << slowpoke_timeout_ms << "[ms]" << std::endl;

	for (size_t i = 0; i < test_repeat_count; ++i)
	{
		time_test_multi_thread(operation_count, slowpoke_timeout_ms);
	}
	for (size_t i = 0; i < test_repeat_count; ++i)
	{
		time_test_single_thread(operation_count, slowpoke_timeout_ms);
	}
	std::cout << std::endl;
}

TEST(readers_writer_lock, thread_count_time_test)
{
	time_test(2, 10, 1);
	time_test(5, 10, 1);
	time_test(10, 10, 1);
	time_test(20, 10, 1);
	time_test(50, 10, 1);
	time_test(100, 10, 1);
}

TEST(readers_writer_lock, task_duration_time_test)
{
	time_test(2, 100, 1);
	time_test(5, 100, 1);
	time_test(10, 100, 1);
	time_test(20, 100, 1);
	time_test(50, 100, 1);
	time_test(100, 100, 1);
}

