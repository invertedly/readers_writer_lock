#include "pch.h"

#include "../readers_writer_lock/reader_lock.h"
#include "../readers_writer_lock/rwlock.h"
#include "../readers_writer_lock/rwlock_exception.h"

#include <memory>

TEST(rwlock, construct)
{
	rwlock::rwlock lock{};

	rwlock::rwlock& lock_ref = lock;
	//rwlock::reader_lock reader_lock{ lock , false, -1 };
	//EXPECT_FALSE(reader_lock.is_locked());

	//auto ptr1 = std::make_shared<rwlock::reader_lock>(rwlock::reader_lock{lock, false});
	auto ptr2 = std::make_shared<rwlock::rwlock>( rwlock::rwlock{});
	//EXPECT_TRUE(ptr2);
	//
	//EXPECT_TRUE(lock.write_lock());

	//EXPECT_FALSE(lock.is_write_locked_in_this_thread());
	//EXPECT_TRUE(lock.is_write_locked_in_this_thread());
	//ptr2->is_write_locked_in_this_thread();
	//ptr2->write_lock();

	EXPECT_FALSE(ptr2->is_write_locked_for_this_thread());
	EXPECT_TRUE(ptr2->write_lock());
	EXPECT_THROW(ptr2->write_lock(), rwlock::rwlock_exception);
	EXPECT_TRUE(ptr2->is_write_locked_for_this_thread());
	EXPECT_FALSE(ptr2->read_lock());
	EXPECT_NO_THROW(ptr2->write_unlock());
	EXPECT_FALSE(ptr2->is_write_locked_for_this_thread());
}

//#include <chrono>
//#include <mutex>
//#include <thread>
//#include <iostream>
//using namespace std;
//mutex m;
//void funcA()
//{
//	cout << "FuncA Before lock" << endl;
//	unique_lock<mutex> mLock(m);
//	//thread 2
//	cout << "FuncA After lock" << endl;
//	std::chrono::milliseconds dura(500);//make sure thread is running
//	std::this_thread::sleep_for(dura);        //this_thread::sleep_for(dura);
//	cout << "FuncA After sleep" << endl;
//}
//
//int main(int argc, char* argv[])
//{
//	cout << "Main before lock" << endl;
//	unique_lock<mutex> mLock(m);
//	auto a = std::thread(funcA);
//	std::chrono::milliseconds dura(1000);//make sure thread is running
//	std::this_thread::sleep_for(dura);        //this_thread::sleep_for(dura);
//	mLock.unlock();//Unlocks thread 2's lock?
//	cout << "Main After unlock" << endl;
//	a.join();
//	cout << "Main after a.join" << endl;
//	return 0;
//}