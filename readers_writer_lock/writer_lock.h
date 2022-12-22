#pragma once

#include "rwlock.h"

namespace rwlock
{
    class writer_lock final
    {
        std::shared_ptr<rwlock> rwlock_ptr_;
        mutable std::mutex internal_mutex_;
    public:
        writer_lock()                                            = delete;
        writer_lock(const std::shared_ptr<rwlock>&)              = delete;
        writer_lock(std::shared_ptr<rwlock>&&)                   = delete;
        writer_lock& operator=(const std::shared_ptr<rwlock>&)   = delete;
        writer_lock& operator=(std::shared_ptr<rwlock>&&)        = delete;

        explicit writer_lock(
            std::shared_ptr<rwlock> parent, 
            bool auto_lock = true, 
            int64_t timeout_ms = -1
        );
        ~writer_lock();
        bool is_locked() const;
        bool lock(int64_t timeout_ms = -1);
        void unlock();
    };
}
