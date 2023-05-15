// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_LOGGING_HPP
#define HSMCPP_LOGGING_HPP

#if defined(HSM_LOGGING_MODE_NICE)
#define HSM_USE_CONSOLE_ERRORS
#elif defined(HSM_LOGGING_MODE_DEBUG_OFF)
#define HSM_DISABLE_DEBUG_TRACES
#define HSM_USE_CONSOLE_TRACES
#elif defined(HSM_LOGGING_MODE_STRICT_VERBOSE)
#define HSM_USE_CONSOLE_TRACES
#define HSM_EXIT_ON_FATAL
#elif defined(HSM_LOGGING_MODE_STRICT)
#define HSM_USE_CONSOLE_ERRORS
#define HSM_EXIT_ON_FATAL
#else // HSM_LOGGING_MODE_OFF
#define HSM_DISABLE_TRACES
#endif

// ---------------------------------------------------------------------------------
// This macroses needs to de defined in each CPP files
// #undef HSM_TRACE_CLASS
// #define HSM_TRACE_CLASS                         "default"

#ifndef HSM_DISABLE_TRACES
 #ifdef PLATFORM_ARDUINO
  #include <Arduino.h>
 #elif !defined(PLATFORM_FREERTOS)
  #include <sys/syscall.h>
  #include <unistd.h>
 #else
  // NOTE: used only for logging in debug mode. in release mode HSM_DISABLE_TRACES is defined so this include is ignored
  // cppcheck-suppress misra-c2012-21.6
  #include <stdio.h>
 #endif

  // ---------------------------------------------------------------------------------
  //                     PRIVATE MACROSES
  // Don't use this in the code. It's for internal usage only
  extern int g_hsm_traces_pid;

 #ifdef PLATFORM_ARDUINO
  #define HSM_TRACE_CALL_COMMON()
  #define HSM_TRACE_INIT()
 #elif !defined(PLATFORM_FREERTOS)
  #define HSM_TRACE_CALL_COMMON()             const int _tid = syscall(__NR_gettid); (void)_tid
  #define HSM_TRACE_INIT()                    if (0 == g_hsm_traces_pid){ g_hsm_traces_pid = getpid(); }
 #else
  #define HSM_TRACE_CALL_COMMON()             const int _tid = 0; (void)_tid
  #define HSM_TRACE_INIT()
 #endif

  #ifdef PLATFORM_ARDUINO
    void serialPrintf(const char* fmt, ...);

    #define HSM_TRACE_CONSOLE_FORCE(msg, ...) \
        serialPrintf((const char*)F("[HSM] %s::%s: " msg), HSM_TRACE_CLASS, __func__,## __VA_ARGS__)
  #else
  #define HSM_TRACE_CONSOLE_FORCE(msg, ...) \
      printf("[PID:%d, TID:%d] %s::%s: " msg "\n", g_hsm_traces_pid, _tid, HSM_TRACE_CLASS, __func__,## __VA_ARGS__)
  #endif

  #ifdef HSM_USE_CONSOLE_TRACES
    #define HSM_TRACE_CONSOLE(msg, ...)         HSM_TRACE_CONSOLE_FORCE(msg,## __VA_ARGS__)
    #define HSM_TRACE_ERROR_CONSOLE(msg, ...)
  #else
    #define HSM_TRACE_CONSOLE(msg, ...)
    #ifdef HSM_USE_CONSOLE_ERRORS
      #define HSM_TRACE_ERROR_CONSOLE(msg, ...)   HSM_TRACE_CONSOLE_FORCE(msg,## __VA_ARGS__)
    #else
      #define HSM_TRACE_ERROR_CONSOLE(msg, ...)
    #endif
  #endif

  #define HSM_TRACE_COMMON(msg, ...)          HSM_TRACE_CONSOLE(msg,## __VA_ARGS__)
  // ---------------------------------------------------------------------------------

  #define HSM_TRACE_PREINIT()                 int g_hsm_traces_pid = (0);
  #define HSM_TRACE(msg, ...)                 HSM_TRACE_COMMON(msg,## __VA_ARGS__)
  #define HSM_TRACE_WARNING(msg, ...)         HSM_TRACE_COMMON("[WARNING] " msg,## __VA_ARGS__)
  #define HSM_TRACE_ERROR(msg, ...)           HSM_TRACE_COMMON("[ERROR] " msg,## __VA_ARGS__); \
                                                  HSM_TRACE_ERROR_CONSOLE("[ERROR] " msg,## __VA_ARGS__)
  #if defined(HSM_EXIT_ON_FATAL) && !defined(PLATFORM_FREERTOS)
    #define HSM_TRACE_FATAL(msg, ...)       HSM_TRACE_COMMON("[FATAL] " msg,## __VA_ARGS__); \
                                                HSM_TRACE_ERROR_CONSOLE("[FATAL] " msg,## __VA_ARGS__); \
                                                exit(1)
  #else
    #define HSM_TRACE_FATAL(msg, ...)       HSM_TRACE_COMMON("[FATAL] " msg,## __VA_ARGS__); \
                                                HSM_TRACE_ERROR_CONSOLE("[FATAL] " msg,## __VA_ARGS__)
  #endif  // HSM_EXIT_ON_FATAL
  #define HSM_TRACE_CALL()                    HSM_TRACE_CALL_COMMON(); \
                                              HSM_TRACE(" was called")
  #define HSM_TRACE_CALL_ARGS(msg, ...)       HSM_TRACE_CALL_COMMON(); \
                                              HSM_TRACE(msg,## __VA_ARGS__)
  #define HSM_TRACE_DEF()                     HSM_TRACE_CALL_COMMON()
#else // PLATFROM_FREERTOS
  #define HSM_TRACE_PREINIT()             int g_hsm_traces_pid = (0);
  #define HSM_TRACE_INIT()
  #define HSM_TRACE(msg, ...)
  #define HSM_TRACE_WARNING(msg, ...)
  #define HSM_TRACE_ERROR(msg, ...)
  #define HSM_TRACE_FATAL(msg, ...)
  #define HSM_TRACE_CALL()
  #define HSM_TRACE_CALL_ARGS(msg, ...)
  #define HSM_TRACE_DEF()
#endif // HSM_DISABLE_TRACES

#if !defined(HSM_DISABLE_DEBUG_TRACES) && !defined(HSM_DISABLE_TRACES)
  #define HSM_TRACE_DEBUG(msg, ...) HSM_TRACE_COMMON("[DEBUG] " msg,## __VA_ARGS__)
  // Should be a first line in any function that needs traces
  #define HSM_TRACE_CALL_DEBUG()              HSM_TRACE_CALL_COMMON(); \
                                              HSM_TRACE_DEBUG(" was called")

  #define HSM_TRACE_CALL_DEBUG_ARGS(msg, ...) HSM_TRACE_CALL_COMMON(); \
                                              HSM_TRACE_DEBUG(msg,## __VA_ARGS__)

  #define HSM_TRACE_CALL_RESULT(msg, ...)     HSM_TRACE_DEBUG(" => " msg,## __VA_ARGS__)
  #define HSM_TRACE_LINE()                    HSM_TRACE_DEBUG("line:%d", __LINE__)
#else  // !defined(HSM_DISABLE_DEBUG_TRACES) && !defined(HSM_DISABLE_TRACES)
  #define HSM_TRACE_DEBUG(msg, ...)

  #ifndef HSM_DISABLE_TRACES
    #define HSM_TRACE_CALL_DEBUG()              HSM_TRACE_CALL_COMMON()
    #define HSM_TRACE_CALL_DEBUG_ARGS(msg, ...) HSM_TRACE_CALL_COMMON()
  #else
    #define HSM_TRACE_CALL_DEBUG()
    #define HSM_TRACE_CALL_DEBUG_ARGS(msg, ...)
  #endif

  #define HSM_TRACE_CALL_RESULT(msg, ...)
  #define HSM_TRACE_LINE()
#endif  // !defined(HSM_DISABLE_DEBUG_TRACES) && !defined(HSM_DISABLE_TRACES)

// ---------------------------------------------------------------------------------
//                          HELPERS
#ifndef SC2INT
  #define SC2INT(v)               static_cast<int>(v)
#endif
#ifndef BOOL2INT
  #define BOOL2INT(v) static_cast<int>(v)
#endif
#ifndef BOOL2STR
  #define BOOL2STR(v) ((v) ? "true" : "false")
#endif

#ifndef DEF2STR
  // NOTE: convenience macros for logging. logs are not used in relase mode
  // cppcheck-suppress misra-c2012-20.10
  #define DEF2STR_INTERNAL(s) #s
  #define DEF2STR(s) DEF2STR_INTERNAL(s)
#endif

#endif  // HSMCPP_LOGGING_HPP
