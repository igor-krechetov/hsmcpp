// set an env var for one test and restore it afterwards.
#ifndef HSMCPP_TESTS_UTILS_SCOPEDENV_HPP
#define HSMCPP_TESTS_UTILS_SCOPEDENV_HPP

#include <string>

class ScopedEnv {
public:
    // set `name` to `value` for the life of the instance, restoring/unsetting on dtor.
    ScopedEnv(const char* name, const std::string& value);
    // unset `name` for the life of the instance, restoring it on dtor if it was set.
    explicit ScopedEnv(const char* name);
    ~ScopedEnv();

private:
    void set(const std::string& v);
    void unset();

private:
    std::string mName;
    std::string mPrev;
    bool mHad = false;
};

#endif  // HSMCPP_TESTS_UTILS_SCOPEDENV_HPP
