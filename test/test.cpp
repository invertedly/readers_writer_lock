#include "pch.h"

#include "../readers_writer_lock/reader_lock.h"
#include "../readers_writer_lock/rwlock.h"
#include "../readers_writer_lock/rwlock_exception.h"
#include "../readers_writer_lock/writer_lock.h"

#include <memory>
#include <chrono>
#include <future>

TEST(rwlock, no_lock)
{
	auto rwlock_ptr = std::make_shared<rwlock::rwlock>(rwlock::rwlock{});

	EXPECT_FALSE(rwlock_ptr->is_write_locked_for_this_thread());
	EXPECT_FALSE(rwlock_ptr->is_read_locked_for_this_thread());

	EXPECT_THROW(rwlock_ptr->read_unlock(), rwlock::rwlock_exception);
	EXPECT_THROW(rwlock_ptr->write_unlock(), rwlock::rwlock_exception);
}

TEST(rwlock, read_lock)
{
	auto rwlock_ptr = std::make_shared<rwlock::rwlock>(rwlock::rwlock{});

	EXPECT_TRUE(rwlock_ptr->read_lock());

	EXPECT_FALSE(rwlock_ptr->is_write_locked_for_this_thread());
	EXPECT_TRUE(rwlock_ptr->is_read_locked_for_this_thread());
	EXPECT_THROW(rwlock_ptr->read_lock(), rwlock::rwlock_exception);
	EXPECT_FALSE(rwlock_ptr->write_lock());
	EXPECT_THROW(rwlock_ptr->write_unlock(), rwlock::rwlock_exception);

	std::jthread read_locker([&]()
		{
			EXPECT_TRUE(rwlock_ptr->read_lock());

			EXPECT_FALSE(rwlock_ptr->is_write_locked_for_this_thread());
			EXPECT_TRUE(rwlock_ptr->is_read_locked_for_this_thread());
			EXPECT_THROW(rwlock_ptr->read_lock(), rwlock::rwlock_exception);
			EXPECT_FALSE(rwlock_ptr->write_lock());
			EXPECT_THROW(rwlock_ptr->write_unlock(), rwlock::rwlock_exception);

			EXPECT_NO_THROW(rwlock_ptr->read_unlock());

			EXPECT_FALSE(rwlock_ptr->is_write_locked_for_this_thread());
			EXPECT_FALSE(rwlock_ptr->is_read_locked_for_this_thread());
		});

	read_locker.join();

	EXPECT_FALSE(rwlock_ptr->is_write_locked_for_this_thread());
	EXPECT_TRUE(rwlock_ptr->is_read_locked_for_this_thread());
	EXPECT_FALSE(rwlock_ptr->write_lock());

	EXPECT_NO_THROW(rwlock_ptr->read_unlock(), rwlock::rwlock_exception);

	EXPECT_FALSE(rwlock_ptr->is_write_locked_for_this_thread());
	EXPECT_FALSE(rwlock_ptr->is_read_locked_for_this_thread());
	EXPECT_TRUE(rwlock_ptr->write_lock());

	EXPECT_NO_THROW(rwlock_ptr->write_unlock());
}

TEST(rwlock, write_lock)
{
	auto rwlock_ptr = std::make_shared<rwlock::rwlock>(rwlock::rwlock{});

	EXPECT_TRUE(rwlock_ptr->write_lock());

	EXPECT_THROW(rwlock_ptr->write_lock(), rwlock::rwlock_exception);
	EXPECT_TRUE(rwlock_ptr->is_write_locked_for_this_thread());
	EXPECT_FALSE(rwlock_ptr->read_lock());
	EXPECT_THROW(rwlock_ptr->read_unlock(), rwlock::rwlock_exception);
	EXPECT_TRUE(rwlock_ptr->is_write_locked_for_this_thread());
	EXPECT_FALSE(rwlock_ptr->is_read_locked_for_this_thread());

	std::jthread write_locker([&]()
		{
			EXPECT_TRUE(rwlock_ptr->write_lock(1000000));

			EXPECT_TRUE(rwlock_ptr->is_write_locked_for_this_thread());
			EXPECT_FALSE(rwlock_ptr->is_read_locked_for_this_thread());
			EXPECT_THROW(rwlock_ptr->write_lock(), rwlock::rwlock_exception);
			EXPECT_FALSE(rwlock_ptr->read_lock());
			EXPECT_THROW(rwlock_ptr->read_unlock(), rwlock::rwlock_exception);

			EXPECT_NO_THROW(rwlock_ptr->write_unlock());

			EXPECT_FALSE(rwlock_ptr->is_write_locked_for_this_thread());
			EXPECT_FALSE(rwlock_ptr->is_read_locked_for_this_thread());
		});

	EXPECT_TRUE(rwlock_ptr->is_write_locked_for_this_thread());
	EXPECT_NO_THROW(rwlock_ptr->write_unlock(), rwlock::rwlock_exception);
	EXPECT_FALSE(rwlock_ptr->is_write_locked_for_this_thread());
	write_locker.join();

	EXPECT_FALSE(rwlock_ptr->is_write_locked_for_this_thread());
	EXPECT_FALSE(rwlock_ptr->is_read_locked_for_this_thread());
	EXPECT_TRUE(rwlock_ptr->write_lock());
	EXPECT_NO_THROW(rwlock_ptr->write_unlock());

	EXPECT_FALSE(rwlock_ptr->is_write_locked_for_this_thread());
	EXPECT_FALSE(rwlock_ptr->is_read_locked_for_this_thread());

	EXPECT_TRUE(rwlock_ptr->read_lock());
	EXPECT_NO_THROW(rwlock_ptr->read_unlock());
}

TEST(readers_writer_lock, construct)
{
	auto rwlock_ptr = std::make_shared<rwlock::rwlock>(rwlock::rwlock{});

	rwlock::reader_lock reader1(rwlock_ptr, false);
	EXPECT_FALSE(reader1.is_locked());
	rwlock::writer_lock writer1(rwlock_ptr, false);
	EXPECT_FALSE(writer1.is_locked());

	rwlock::reader_lock reader2(rwlock_ptr, true);
	EXPECT_TRUE(reader2.is_locked());
	EXPECT_NO_THROW(reader2.unlock());
	EXPECT_THROW(reader2.unlock(), rwlock::rwlock_exception);
	EXPECT_FALSE(reader2.is_locked());

	rwlock::writer_lock writer2(rwlock_ptr, true);
	EXPECT_TRUE(writer2.is_locked());
	EXPECT_NO_THROW(writer2.unlock());
	EXPECT_THROW(writer2.unlock(), rwlock::rwlock_exception);
	EXPECT_FALSE(writer2.is_locked());
}

TEST(readers_writer_lock, lock)
{
	auto rwlock_ptr = std::make_shared<rwlock::rwlock>(rwlock::rwlock{});

	rwlock::reader_lock reader1{rwlock_ptr, false};

	EXPECT_TRUE(reader1.lock());
	EXPECT_TRUE(reader1.is_locked());

	rwlock::writer_lock writer1(rwlock_ptr, false);
	EXPECT_FALSE(writer1.lock());
	EXPECT_FALSE(writer1.is_locked());

	std::jthread locker1([](std::shared_ptr<rwlock::rwlock> rwlock_ptr)
		{
			rwlock::reader_lock reader2{ rwlock_ptr, false };
			EXPECT_TRUE(reader2.lock());
			EXPECT_TRUE(reader2.is_locked());
			EXPECT_NO_THROW(reader2.unlock());

			rwlock::writer_lock writer2{ rwlock_ptr, false };
			EXPECT_FALSE(writer2.lock());
		}, rwlock_ptr);

	locker1.join();

	EXPECT_NO_THROW(reader1.unlock());
	EXPECT_TRUE(writer1.lock());
	EXPECT_TRUE(writer1.is_locked());
}

class rwlocked_string final
{
	std::string data_;
	mutable std::shared_ptr<rwlock::rwlock> rwlock_ptr_;

public:
	rwlocked_string(const std::string& buf = {}) : rwlock_ptr_(new rwlock::rwlock)
	{
		write(buf, 0);
	}

	std::string read(const int64_t timeout = 0) const
	{
		rwlock::reader_lock lock(rwlock_ptr_, true, timeout);

		while (!lock.is_locked())
		{
			lock.lock(timeout);
		}

		return data_;
	}

	void write(const std::string& buf, const int64_t timeout = 0)
	{
		rwlock::writer_lock lock(rwlock_ptr_, true, timeout);

		while (!lock.is_locked())
		{
			lock.lock(timeout);
		}

		data_ = buf;
	}
};


void multithreading_test(
	enum class std::launch type = std::launch::async,
	int64_t timeout = 0
)
{
	rwlocked_string shared_string{"initial"};

	//for (int i = 0; i < 10000; ++i)
	//{
	//	std::jthread reader(
	//		&rwlocked_string::read,
	//		shared_string,
	//		5
	//	);
	//}

	//std::jthread writer(
	//	&rwlocked_string::write,
	//	shared_string,
	//	5
	//);
	//std::jthread write1(
	//	&rwlocked_string::write,
	//	shared_string,
	//	"write1",
	//	timeout
	//);

	//write1.join();

	//std::future<std::string> read1 = std::async(
	//	type,
	//	&rwlocked_string::read,
	//	shared_string,
	//	timeout
	//);

	//std::future<std::string> read2 = std::async(
	//	type,
	//	&rwlocked_string::read,
	//	shared_string,
	//	timeout
	//);

	//std::future<std::string> read3 = std::async(
	//	type,
	//	&rwlocked_string::read,
	//	shared_string,
	//	timeout
	//);

	//std::future<std::string> read4 = std::async(
	//	type,
	//	&rwlocked_string::read,
	//	shared_string,
	//	timeout
	//);

	//std::future<std::string> read5 = std::async(
	//	type,
	//	&rwlocked_string::read,
	//	shared_string,
	//	0
	//);
	//std::cout << read1.get() << std::endl;
	//std::cout << read2.get() << std::endl;
	//std::cout << read3.get() << std::endl;
	//std::cout << read4.get() << std::endl;
	//std::cout << read5.get() << std::endl;

	/*EXPECT_TRUE(read1.get() == std::string{"write1"});
	EXPECT_TRUE(read2.get() == std::string{"write1"});
	EXPECT_TRUE(read3.get() == std::string{"write1"});
	EXPECT_TRUE(read4.get() == std::string{"write1"});
	EXPECT_TRUE(read5.get() == std::string{"write1"});*/
}


TEST(readers_writer_lock, time_test)
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	rwlocked_string shared_string{ "initial" };

	for (int i = 0; i < 100; ++i)
	{
		std::jthread reader(
			&rwlocked_string::read,
			shared_string,
			5
		);
		reader.detach();
	}

	std::jthread writer(
		&rwlocked_string::write,
		shared_string,
		"final",
		5
	);

	writer.join();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[ms]" << std::endl;
}


TEST(readers_writer_lock, multi_thread_time_test)
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	rwlocked_string shared_string{ "initial" };

	for (int i = 0; i < 100; ++i)
	{
		//shared_string.read(5);

		std::jthread reader(
			&rwlocked_string::read,
			shared_string,
			5
		);
		reader.join();
	}

	shared_string.write("final", 5);

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[ms]" << std::endl;
}