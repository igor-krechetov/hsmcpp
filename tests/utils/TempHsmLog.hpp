// test-owned temp log file for debug-logging round-trip.
#ifndef HSMCPP_TESTS_UTILS_TEMPHSMLOG_HPP
#define HSMCPP_TESTS_UTILS_TEMPHSMLOG_HPP

#include <cstdint>
#include <string>

class TempHsmLog {
public:
    TempHsmLog();
    ~TempHsmLog();

    const std::string& path() const;
    bool exists() const;
    std::uintmax_t size() const;
    std::string read() const;
    void remove() const;

private:
    std::string mPath;
};

#endif  // HSMCPP_TESTS_UTILS_TEMPHSMLOG_HPP
