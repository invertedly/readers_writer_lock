#include "rwlock.h"

namespace rwlock
{
	rwlock::rwlock()
		: shared_timed_mutex_ptr_(new std::shared_timed_mutex)
	{
	}
	
	rwlock::rwlock(rwlock&& other) noexcept
	{
		std::scoped_lock rhs_lock(other.internal_mutex_);

		shared_timed_mutex_ptr_ = std::move(other.shared_timed_mutex_ptr_);
	}
	
	rwlock& rwlock::operator=(rwlock&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}
	
		std::scoped_lock lock(internal_mutex_, other.internal_mutex_);

		shared_timed_mutex_ptr_ = std::move(other.shared_timed_mutex_ptr_);

		return *this;
	}


	bool rwlock::read_lock(const int64_t timeout_ms) const
	{
		std::scoped_lock lock(internal_mutex_);

		return shared_timed_mutex_ptr_->try_lock_shared_for(
			std::chrono::duration<uint64_t, std::milli>(timeout_ms)
		);
	}

	void rwlock::read_unlock() const
	{
		std::scoped_lock lock(internal_mutex_);
	
		shared_timed_mutex_ptr_->unlock();
	}
	
	bool rwlock::write_lock(const int64_t timeous_ms)
	{
		std::scoped_lock lock(internal_mutex_);
	
		return shared_timed_mutex_ptr_->try_lock_for(
			std::chrono::duration<uint64_t, std::milli>(timeous_ms)
		);
	}
	
	void rwlock::write_unlock()
	{
		std::scoped_lock lock(internal_mutex_);
	
		shared_timed_mutex_ptr_->unlock();
	}
}
