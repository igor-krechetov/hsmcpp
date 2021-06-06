#include <stdio.h>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

int main(const int argc, const char**argv)
{
    printf("Off\n");
    std::this_thread::sleep_for(1000ms);
    printf("On\n");
    std::this_thread::sleep_for(1000ms);
    return 0;
}