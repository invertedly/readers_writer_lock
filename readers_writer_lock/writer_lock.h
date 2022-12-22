#pragma once

#include "rwlock.h"

namespace rwlock
{
    class writer_lock final
    {
        rwlock& rwlock_;
        mutable std::mutex internal_mutex_;
    public:
        writer_lock()                           = delete;
        writer_lock(const rwlock&)              = delete;
        writer_lock(rwlock&&)                   = delete;
        writer_lock& operator=(const rwlock&)   = delete;
        writer_lock& operator=(rwlock&&)        = delete;

        explicit writer_lock(rwlock& parent, bool auto_lock = true, int64_t timeout_ms = -1);
        ~writer_lock();
        bool is_locked() const;
        bool lock(int64_t timeout_ms = -1);
        void unlock();
    };
}
