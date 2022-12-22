#pragma once

#include "rwlock.h"

namespace rwlock
{
    class reader_lock final
    {
        std::shared_ptr<rwlock> rwlock_ptr_;
        mutable std::mutex internal_mutex_;
    public:
        reader_lock()                                            = delete;
        reader_lock(const std::shared_ptr<rwlock>&)              = delete;
        reader_lock(std::shared_ptr<rwlock>&&)                   = delete;
        reader_lock& operator=(const std::shared_ptr<rwlock>&)   = delete;
        reader_lock& operator=(std::shared_ptr<rwlock>&&)        = delete;

        explicit reader_lock(
            std::shared_ptr<rwlock> parent, 
            bool auto_lock = true, 
            int64_t timeout_ms = -1
        );
        ~reader_lock();
        [[nodiscard]] bool is_locked() const;
        bool lock(int64_t timeout_ms = -1);
        void unlock();
    };
}
