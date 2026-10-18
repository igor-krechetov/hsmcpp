// RAII guard that deletes a fixed-path file on construction and scope exit.
// Used for paths a test cannot choose (e.g. the compiled-in default dump path
// "./dump.hsmlog"), where TempHsmLog's self-chosen unique path does not apply.
// Mirrors TempHsmLog's teardown guarantee for a caller-provided path.
#ifndef HSMCPP_TESTS_UTILS_SCOPEDFILEGUARD_HPP
#define HSMCPP_TESTS_UTILS_SCOPEDFILEGUARD_HPP

#include <string>

class ScopedFileGuard {
public:
    explicit ScopedFileGuard(std::string path);
    ~ScopedFileGuard();

    void remove() const;

private:
    std::string mPath;
};

#endif  // HSMCPP_TESTS_UTILS_SCOPEDFILEGUARD_HPP
