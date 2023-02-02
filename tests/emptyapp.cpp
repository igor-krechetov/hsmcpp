#include <stdio.h>

#include <chrono>
#include <thread>

int main(const int argc, const char** argv) {
    printf("Off\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    printf("On\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return 0;
}