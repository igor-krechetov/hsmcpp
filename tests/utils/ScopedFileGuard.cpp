#include "utils/ScopedFileGuard.hpp"

#include <filesystem>
#include <system_error>
#include <utility>

ScopedFileGuard::ScopedFileGuard(std::string path)
    : mPath(std::move(path)) {
    remove();  // ensure a clean "new log" state before the test writes
}

ScopedFileGuard::~ScopedFileGuard() {
    remove();
}

void ScopedFileGuard::remove() const {
    std::error_code ec;
    std::filesystem::remove(mPath, ec);
}
