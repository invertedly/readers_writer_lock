#pragma once

#include <shared_mutex>
#include <unordered_set>

namespace rwlock
{
	class rwlock final
	{
        std::unordered_set<std::thread::id> reader_thread_id_;
        std::thread::id writer_thread_id_;

		std::unique_ptr<std::shared_mutex> external_mutex_;
		std::unique_ptr<std::mutex> internal_mutex_;

        std::condition_variable condition_variable_;

        bool try_write_lock()   noexcept;
        bool try_read_lock()    noexcept;

	public: 
        explicit rwlock();

        rwlock(const rwlock&)               = delete;
        rwlock& operator=(const rwlock&)    = delete;

        rwlock(rwlock&& other)              noexcept;
        rwlock& operator=(rwlock&& other)   noexcept;

        ~rwlock() = default;

        bool read_lock(int64_t timeout_ms = 0);
        void read_unlock();

        bool write_lock(int64_t timeout_ms = 0);
        void write_unlock();

        [[nodiscard]] bool is_write_locked_for_this_thread() const;
        [[nodiscard]] bool is_read_locked_for_this_thread()  const;
	};
}
