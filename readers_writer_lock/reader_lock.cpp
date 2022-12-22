#include "reader_lock.h"

namespace rwlock
{
	reader_lock::reader_lock(
		std::shared_ptr<rwlock> parent, 
		const bool auto_lock, 
		const int64_t timeout_ms
	)
		: rwlock_ptr_(std::move(parent))
	{
		std::scoped_lock lock(internal_mutex_);
		if (auto_lock)
		{
			rwlock_ptr_->read_lock(timeout_ms);
		}
	}

	reader_lock::~reader_lock()
	{
		std::scoped_lock lock(internal_mutex_);
		if (rwlock_ptr_->is_read_locked_for_this_thread())
		{
			rwlock_ptr_->read_unlock();
		}
	}

	bool reader_lock::is_locked() const
	{
		std::scoped_lock lock(internal_mutex_);
		return rwlock_ptr_->is_read_locked_for_this_thread();
	}

	bool reader_lock::lock(const int64_t timeout_ms)
	{
		std::scoped_lock lock(internal_mutex_);
		return rwlock_ptr_->read_lock(timeout_ms);
	}

	void reader_lock::unlock()
	{
		std::scoped_lock lock(internal_mutex_);
		rwlock_ptr_->read_unlock();
	}
}
