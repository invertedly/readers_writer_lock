#pragma once

#include <shared_mutex>
#include <unordered_set>

namespace rwlock
{
	class rwlock final
	{
		std::unique_ptr<std::shared_timed_mutex> shared_timed_mutex_ptr_;
		mutable std::mutex internal_mutex_;

	public: 
        explicit rwlock();

        rwlock(const rwlock&)               = delete;
        rwlock& operator=(const rwlock&)    = delete;

        rwlock(rwlock&& other)              noexcept;
        rwlock& operator=(rwlock&& other)   noexcept;

        ~rwlock() = default;

        bool read_lock(int64_t timeout_ms = -1) const;
        void read_unlock() const;

        bool write_lock(int64_t timeous_ms = -1);
        void write_unlock();
	};
}
