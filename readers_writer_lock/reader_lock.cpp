#include "reader_lock.h"

namespace rwlock
{
	reader_lock::reader_lock(
		rwlock& parent, 
		const bool auto_lock, 
		const int64_t timeout_ms
	)
		: rwlock_(parent),
		  is_locked_(false)
	{
		if (auto_lock)
		{
			is_locked_ =  rwlock_.read_lock(timeout_ms);
		}
	}

	reader_lock::~reader_lock()
	{
		if (is_locked_)
		{
			rwlock_.read_unlock();
		}
	}

	bool reader_lock::is_locked() const
	{
		return is_locked_;
	}

	bool reader_lock::lock(const int64_t timeout_ms)
	{
		is_locked_ = rwlock_.read_lock(timeout_ms);
		return is_locked_;
	}

	void reader_lock::unlock()
	{
		rwlock_.read_unlock();
		is_locked_ = false;
	}
}
