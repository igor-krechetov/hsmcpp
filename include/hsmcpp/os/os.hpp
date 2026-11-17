// Copyright (C) 2022 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_OS_HPP
#define HSMCPP_OS_OS_HPP

// TODO: check if there are better ways to detect FreeRTOS
#if defined(PLATFORM_FREERTOS) || defined(INC_FREERTOS_H)
  #define FREERTOS_AVAILABLE (1)
#elif defined(PLATFORM_POSIX) || defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)) || defined(__linux__)
  #define POSIX_AVAILABLE (1)
  #define STL_AVAILABLE (1)
#elif defined(PLATFORM_WINDOWS) || defined(WIN32)
  #define STL_AVAILABLE (1)
#elif defined(ARDUINO) && !defined(PLATFORM_ARDUINO)
  #define PLATFORM_ARDUINO (1)
#endif

// ================================================================================================
// OS abstraction backend resolution
//
// The os/ layer is split into capability DOMAINS (threading, interrupts, ... with filesystem /
// network / logging to follow) and BACKENDS that implement them. A public selector header
// (e.g. os/Mutex.hpp) stays at a stable path and simply includes the backend chosen here, so the
// decision lives in exactly one place and adding a primitive or a whole domain never duplicates
// selection logic.
//
// The two axes are orthogonal: the THREADING backend may be overridden to a single-threaded
// no-op build (HSM_DISABLE_THREADSAFETY) independently of the platform, while the INTERRUPTS
// backend always follows the real platform and is never no-op'd -- an ISR can still preempt a
// single-threaded program, so interrupt masking must stay real.

// Arduino has no threads, so thread-safety support is always disabled there. Arduino IDE gives
// library authors no way to pass compiler defines, so it is set here. Every hsmcpp translation
// unit (library sources and application code) includes this header before reading the macro, so
// the definition stays consistent. Non-Arduino builds must be configured explicitly (CMake checks).
#if defined(ARDUINO) && !defined(HSM_DISABLE_THREADSAFETY)
  #define HSM_DISABLE_THREADSAFETY (1)
#endif

// ---- threading backend (mutex / atomic flag / condition variable) ----
#if defined(HSM_DISABLE_THREADSAFETY)
  #define HSM_THREADING_BACKEND_DIR singlethread
#elif defined(FREERTOS_AVAILABLE)
  #define HSM_THREADING_BACKEND_DIR freertos
#elif defined(PLATFORM_ARDUINO)
  #define HSM_THREADING_BACKEND_DIR arduino
#elif defined(STL_AVAILABLE)
  #define HSM_THREADING_BACKEND_DIR stl
#else
  #error "hsmcpp: no threading backend available for this platform"
#endif

// ---- interrupts backend (interrupts-free section) -- NEVER no-op'd ----
#if defined(FREERTOS_AVAILABLE)
  #define HSM_INTERRUPTS_BACKEND_DIR freertos
#elif defined(POSIX_AVAILABLE)
  #define HSM_INTERRUPTS_BACKEND_DIR posix
#elif defined(PLATFORM_ARDUINO) || defined(STL_AVAILABLE)
  // arduino and windows share the common header (with platform-specific .cpp)
  #define HSM_INTERRUPTS_BACKEND_DIR common
#else
  #error "hsmcpp: no interrupts backend available for this platform"
#endif

// Build an include path "backends/<domain>/<backend-dir>/<File>" from the resolved tokens.
// Two expansion levels are required so the *_DIR macros expand before stringization.
#define HSM_OS_STR2(x) #x
#define HSM_OS_STR(x) HSM_OS_STR2(x)
#define HSM_OS_BACKEND_HEADER(domain, dir, file) HSM_OS_STR(backends/domain/dir/file)

#define HSM_THREADING_HEADER(file) HSM_OS_BACKEND_HEADER(threading, HSM_THREADING_BACKEND_DIR, file)
#define HSM_INTERRUPTS_HEADER(file) HSM_OS_BACKEND_HEADER(interrupts, HSM_INTERRUPTS_BACKEND_DIR, file)

#if defined(__cpp_exceptions) || defined(_CPPUNWIND) || defined(__EXCEPTIONS)
  #define EXCEPTIONS_ENABLED (1)
#endif

// NOTE: needed for embedded platforms where exceptions are disabled
#ifdef EXCEPTIONS_ENABLED
  #define HSM_TRY try
  #define HSM_CATCH(_x) catch (_x)
#else
  #define HSM_TRY if (true)
  #define HSM_CATCH(_x) if (false)
#endif

#endif  // HSMCPP_OS_OS_HPP