// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_WINDOWS_CRITICALSECTION_HPP
#define HSMCPP_OS_WINDOWS_CRITICALSECTION_HPP

namespace hsmcpp
{

class CriticalSection
{
public:
    CriticalSection();
    ~CriticalSection();

private:
    CriticalSection(const CriticalSection&) = delete;
    CriticalSection& operator=(const CriticalSection&) = delete;
    CriticalSection(CriticalSection&&) = delete;
    CriticalSection& operator=(CriticalSection&&) = delete;
};

} // namespace hsmcpp

#endif // HSMCPP_OS_WINDOWS_CRITICALSECTION_HPP
