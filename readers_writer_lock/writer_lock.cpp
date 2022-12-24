#include "writer_lock.h"

namespace rwlock
{
	writer_lock::writer_lock(
		rwlock& parent,
		const bool auto_lock, 
		const int64_t timeout_ms
	)
		: rwlock_(parent),
		  is_locked_(false)
	{
		if (auto_lock)
		{
			is_locked_ = rwlock_.write_lock(timeout_ms);
		}
	}

	writer_lock::~writer_lock()
	{
		if (is_locked_)
		{
			rwlock_.write_unlock();
		}
	}

	bool writer_lock::is_locked() const
	{
		return is_locked_;
	}

	bool writer_lock::lock(const int64_t timeout_ms)
	{
		is_locked_ = rwlock_.write_lock(timeout_ms);
		return is_locked_;
	}

	void writer_lock::unlock()
	{
		rwlock_.write_unlock();
		is_locked_ = false;
	}
}
