#include "pch.h"

#include "../readers_writer_lock/reader_lock.h"
#include "../readers_writer_lock/rwlock.h"
#include "../readers_writer_lock/rwlock_exception.h"
#include "../readers_writer_lock/writer_lock.h"

#include <memory>
#include <chrono>

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

//class rwlocked_data final
//{
//	std::string data_;
//	mutable std::shared_ptr<rwlock::rwlock> rwlock_ptr_;
//	std::condition_variable_any condition_var_any_;
//public:
//	rwlocked_data() : data_("some data"), rwlock_ptr_(new rwlock::rwlock)
//	{
//	}
//
//	std::string read() const
//	{
//		rwlock::reader_lock lock(rwlock_ptr_, true);
//
//		
//
//		return data_;
//	}
//
//	void write()
//	{
//		rw
//	}
//};

TEST(readers_writer_lock, single_thread_time_test)
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();




	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[ms]" << std::endl;
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;
}


TEST(readers_writer_lock, multi_thread_time_test)
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();



	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[ms]" << std::endl;
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;
}