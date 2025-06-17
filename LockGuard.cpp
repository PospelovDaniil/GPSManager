#include "LockGuard.h"

LockGuard::LockGuard(SemaphoreHandle_t mtx, TickType_t timeout)
    : mutex(mtx), taken(pdFALSE) {
    if (mutex != nullptr) {
        taken = xSemaphoreTake(mutex, timeout);
    }
}

LockGuard::~LockGuard() {
    if (taken == pdTRUE) {
        xSemaphoreGive(mutex);
    }
}