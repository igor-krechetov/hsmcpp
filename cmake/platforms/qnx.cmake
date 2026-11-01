add_definitions(-DPLATFORM_POSIX=1)
add_definitions(-DPLATFORM_QNX=1)

# The top-level CMakeLists sets CMAKE_CXX_EXTENSIONS OFF, so QCC compiles in strict
# ISO mode (-std=c++NN rather than -std=gnu++NN). In that mode the QNX C library does
# not implicitly enable its POSIX/QNX extensions, so declarations such as sigfillset(),
# pthread_sigmask() and access() are #if-gated out of <signal.h>/<unistd.h> even though
# the headers are included (types like sigset_t remain visible, which is why only the
# functions were reported as undeclared). Defining _QNX_SOURCE exposes those POSIX/QNX
# declarations. This is scoped to the QNX platform only and does not affect posix/linux.
add_definitions(-D_QNX_SOURCE)