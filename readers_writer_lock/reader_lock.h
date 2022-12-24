#pragma once

#include "rwlock.h"

namespace rwlock
{
    class reader_lock final
    {
        rwlock& rwlock_;
        bool is_locked_;

    public:
        reader_lock()                           = delete;
        reader_lock(const rwlock&)              = delete;
        reader_lock(rwlock&&)                   = delete;
        reader_lock& operator=(const rwlock&)   = delete;
        reader_lock& operator=(rwlock&&)        = delete;

        explicit reader_lock(
            rwlock& parent, 
            bool auto_lock = true, 
            int64_t timeout_ms = -1
        );
        ~reader_lock();
        [[nodiscard]] bool is_locked() const;
        bool lock(int64_t timeout_ms = -1);
        void unlock();
    };
}
