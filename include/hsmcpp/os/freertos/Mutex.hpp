// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_FREERTOS_MUTEX_HPP
#define HSMCPP_OS_FREERTOS_MUTEX_HPP

#include <FreeRTOS.h>
#include <semphr.h>

#if (configUSE_MUTEXES != 1)
 #error configUSE_MUTEXES feature is required
#endif

namespace hsmcpp
{

class Mutex
{
public:
    Mutex();
    ~Mutex();

    void lock();
    void unlock();

private:
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

private:
    SemaphoreHandle_t mHandle = nullptr;
};

} // namespace hsmcpp

#endif // HSMCPP_OS_FREERTOS_MUTEX_HPP
