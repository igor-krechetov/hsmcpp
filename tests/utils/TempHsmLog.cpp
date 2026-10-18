#include "utils/TempHsmLog.hpp"

#if defined(_WIN32)
    #include <process.h>  // _getpid()
#else
    #include <unistd.h>  // getpid()
#endif

#include <atomic>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <system_error>

#if defined(_WIN32)
    #define HSMCPP_GETPID _getpid
#else
    #define HSMCPP_GETPID getpid
#endif

TempHsmLog::TempHsmLog() {
    // unique path under the system temp dir; include pid + counter to avoid
    // collisions when the same test runs in several dispatcher binaries.
    namespace fs = std::filesystem;
    static std::atomic<int> counter{0};
    
    mPath = (fs::temp_directory_path() /
             ("hsmcpp_test_" + std::to_string(HSMCPP_GETPID()) + "_" + std::to_string(counter++) + ".hsmlog"))
                .string();
    remove();  // ensure "new log" branch in enableHsmDebugging()
}

TempHsmLog::~TempHsmLog() {
    remove();
}

const std::string& TempHsmLog::path() const {
    return mPath;
}

bool TempHsmLog::exists() const {
    return std::filesystem::exists(mPath);
}

std::uintmax_t TempHsmLog::size() const {
    return exists() ? std::filesystem::file_size(mPath) : 0U;
}

std::string TempHsmLog::read() const {
    std::ifstream in(mPath, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

void TempHsmLog::remove() const {
    std::error_code ec;
    std::filesystem::remove(mPath, ec);
}
