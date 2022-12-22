#include "rwlock.h"

#include "rwlock_exception.h"

namespace rwlock
{
	bool rwlock::try_write_lock() noexcept
	{
		const bool write_lock_success = external_mutex_ptr_->try_lock();

		if (write_lock_success)
		{
			writer_thread_id_ = std::this_thread::get_id();
		}

		return write_lock_success;
	}

	bool rwlock::try_read_lock() noexcept
	{
		const bool read_lock_success = external_mutex_ptr_->try_lock_shared();

		if (read_lock_success)
		{
			reader_thread_id_.insert(std::this_thread::get_id());
		}

		return read_lock_success;
	}

	rwlock::rwlock()
		: writer_thread_id_(std::thread::id()),
		  external_mutex_ptr_(new std::shared_mutex),
		  internal_mutex_ptr_(new std::mutex)
	{
	}

	rwlock::rwlock(rwlock&& other) noexcept
	{
		std::scoped_lock rhs_lock(*other.internal_mutex_ptr_);

		reader_thread_id_ = std::move(other.reader_thread_id_);
		writer_thread_id_ = other.writer_thread_id_;
		external_mutex_ptr_ = std::move(other.external_mutex_ptr_);
		internal_mutex_ptr_ = std::move(other.internal_mutex_ptr_);
	}

	rwlock& rwlock::operator=(rwlock&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		std::unique_lock lhs_lock(*internal_mutex_ptr_, std::defer_lock);
		std::unique_lock rhs_lock(*other.internal_mutex_ptr_, std::defer_lock);
		std::lock(lhs_lock, rhs_lock);

		reader_thread_id_ = std::move(other.reader_thread_id_);
		writer_thread_id_ = other.writer_thread_id_;
		external_mutex_ptr_ = std::move(other.external_mutex_ptr_);
		internal_mutex_ptr_ = std::move(other.internal_mutex_ptr_);

		return *this;
	}

	bool rwlock::read_lock(const int64_t timeout_ms)
	{
		std::unique_lock lock(*internal_mutex_ptr_);

		if (reader_thread_id_.contains(std::this_thread::get_id()))
		{
			throw rwlock_exception("thread is already read locked");
		}

		if (writer_thread_id_ != std::thread::id())
		{
			return false;
		}

		if (try_read_lock())
		{
			return true;
		}

		if (timeout_ms > 0)
		{
			condition_variable_.wait_for(
				lock, 
				std::chrono::duration<uint64_t, std::micro>(timeout_ms)
			);
		}

		return try_read_lock();
	}
	void rwlock::read_unlock()
	{
		std::scoped_lock lock(*internal_mutex_ptr_);

		if (reader_thread_id_.contains(std::this_thread::get_id()))
		{
			external_mutex_ptr_->unlock_shared();
			reader_thread_id_.erase(std::this_thread::get_id());
		}
		else
		{
			throw rwlock_exception("thread is not read locked");
		}
	}

	bool rwlock::write_lock(const int64_t timeout_ms)
	{
		std::unique_lock lock(*internal_mutex_ptr_);

		if (writer_thread_id_ == std::this_thread::get_id())
		{
			throw rwlock_exception("thread is already write locked");
		}

		if (writer_thread_id_ != std::thread::id() || !reader_thread_id_.empty())
		{
			return false;
		}

		if (try_write_lock())
		{
			return true;
		}

		if (timeout_ms > 0)
		{
			condition_variable_.wait_for(
				lock, 
				std::chrono::duration<uint64_t, std::micro>(timeout_ms)
			);
		}

		return try_write_lock();
	}

	void rwlock::write_unlock()
	{
		std::scoped_lock lock(*internal_mutex_ptr_);

		if (writer_thread_id_ == std::this_thread::get_id())
		{
			external_mutex_ptr_->unlock();
			writer_thread_id_ = std::thread::id();
		}
		else
		{
			throw rwlock_exception("thread is not write locked");
		}
	}

	bool rwlock::is_write_locked_for_this_thread() const
	{
		std::scoped_lock lock(*internal_mutex_ptr_);
		return writer_thread_id_ == std::this_thread::get_id();
	}

	bool rwlock::is_read_locked_for_this_thread() const
	{
		std::scoped_lock lock(*internal_mutex_ptr_);
		return reader_thread_id_.contains(std::this_thread::get_id());
	}
}
