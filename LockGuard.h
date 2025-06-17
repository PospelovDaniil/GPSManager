#ifndef LOCK_GUARD_H
#define LOCK_GUARD_H

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class LockGuard {
public:
    explicit LockGuard(SemaphoreHandle_t mutex, TickType_t timeout = pdMS_TO_TICKS(100));
    ~LockGuard();

    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;

    inline bool isLocked() const { return taken == pdTRUE; }

private:
    SemaphoreHandle_t mutex;
    BaseType_t taken;
};

#endif // LOCK_GUARD_H