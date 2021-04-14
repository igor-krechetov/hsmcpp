// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_LOGGING_HPP__
#define __HSMCPP_LOGGING_HPP__

#if defined(LOGGING_MODE_NICE)
#define USE_CONSOLE_ERRORS
#elif defined(LOGGING_MODE_OFF)
#define DISABLE_TRACES
#elif defined(LOGGING_MODE_DEBUG_OFF)
#define DISABLE_DEBUG_TRACES
#elif defined(LOGGING_MODE_STRICT_VERBOSE)
#define USE_CONSOLE_TRACES
#define EXIT_ON_FATAL
#else
#define USE_CONSOLE_ERRORS
#define EXIT_ON_FATAL
#endif

// ---------------------------------------------------------------------------------
// This macroses needs to de defined in each CPP files
// #undef __TRACE_CLASS__
// #define __TRACE_CLASS__                         "default"

#ifndef DISABLE_TRACES
  #include <sys/syscall.h>
  #include <unistd.h>

  // ---------------------------------------------------------------------------------
  //                     PRIVATE MACROSES
  // Don't use this in the code. It's for internal usage only
  extern int g_traces_pid;

  #define __TRACE_CALL_COMMON__()             const int _tid = syscall(__NR_gettid)

  #define __TRACE_CONSOLE_FORCE__(msg, ...) \
      printf("[PID:%d, TID:%d] " __TRACE_CLASS__ "::%s: " msg "\n", g_traces_pid, _tid, __func__,## __VA_ARGS__)
  
  #ifdef USE_CONSOLE_TRACES
    #define __TRACE_CONSOLE__(msg, ...)         __TRACE_CONSOLE_FORCE__(msg,## __VA_ARGS__)
    #define __TRACE_ERROR_CONSOLE__(msg, ...)
  #else
    #define __TRACE_CONSOLE__(msg, ...)
    #ifdef USE_CONSOLE_ERRORS
      #define __TRACE_ERROR_CONSOLE__(msg, ...)   __TRACE_CONSOLE_FORCE__(msg,## __VA_ARGS__)
    #else
      #define __TRACE_ERROR_CONSOLE__(msg, ...)
    #endif
  #endif

  #define __TRACE_COMMON__(msg, ...)          __TRACE_CONSOLE__(msg,## __VA_ARGS__)
  // ---------------------------------------------------------------------------------

  #define __TRACE_PREINIT__()                 int g_traces_pid = 0
  #define __TRACE_INIT__()                    if (0 == g_traces_pid){ g_traces_pid = getpid(); }
  #define __TRACE__(msg, ...)                 __TRACE_COMMON__(msg,## __VA_ARGS__)
  #define __TRACE_WARNING__(msg, ...)         __TRACE_COMMON__("[WARNING] " msg,## __VA_ARGS__)
  #define __TRACE_ERROR__(msg, ...)           __TRACE_COMMON__("[ERROR] " msg,## __VA_ARGS__); \
                                              __TRACE_ERROR_CONSOLE__("[ERROR] " msg,## __VA_ARGS__)
  #ifdef EXIT_ON_FATAL
    #define __TRACE_FATAL__(msg, ...)           __TRACE_COMMON__("[FATAL] " msg,## __VA_ARGS__); \
                                                __TRACE_ERROR_CONSOLE__("[FATAL] " msg,## __VA_ARGS__); \
                                                exit(1)
  #else
    #define __TRACE_FATAL__(msg, ...)           __TRACE_COMMON__("[FATAL] " msg,## __VA_ARGS__); \
                                                __TRACE_ERROR_CONSOLE__("[FATAL] " msg,## __VA_ARGS__)
  #endif  // EXIT_ON_FATAL
  #define __TRACE_CALL__()                    __TRACE_CALL_COMMON__(); \
                                              __TRACE__(" was called")
  #define __TRACE_CALL_ARGS__(msg, ...)       __TRACE_CALL_COMMON__(); \
                                              __TRACE__(msg,## __VA_ARGS__)
  #define __TRACE_DEF__()                     __TRACE_CALL_COMMON__()
#else
  #define __TRACE_PREINIT__()
  #define __TRACE_INIT__()
  #define __TRACE__(msg, ...)
  #define __TRACE_WARNING__(msg, ...)
  #define __TRACE_ERROR__(msg, ...)
  #define __TRACE_FATAL__(msg, ...)
  #define __TRACE_CALL__()
  #define __TRACE_CALL_ARGS__(msg, ...)
  #define __TRACE_DEF__()
#endif // DISABLE_TRACES

#if !defined(DISABLE_DEBUG_TRACES) && !defined(DISABLE_TRACES)
  #define __TRACE_DEBUG__(msg, ...) __TRACE_COMMON__("[DEBUG] " msg,## __VA_ARGS__)
  // Should be a first line in any function that needs traces
  #define __TRACE_CALL_DEBUG__()              __TRACE_CALL_COMMON__(); \
                                              __TRACE_DEBUG__(" was called")

  #define __TRACE_CALL_DEBUG_ARGS__(msg, ...) __TRACE_CALL_COMMON__(); \
                                              __TRACE_DEBUG__(msg,## __VA_ARGS__)

  #define __TRACE_CALL_RESULT__(msg, ...)     __TRACE_DEBUG__(" => " msg,## __VA_ARGS__)
  #define __TRACE_LINE__()                    __TRACE_DEBUG__("line:%d", __LINE__)
#else  // !defined(DISABLE_DEBUG_TRACES) && !defined(DISABLE_TRACES)
  #define __TRACE_DEBUG__(msg, ...)

  #ifndef DISABLE_TRACES
    #define __TRACE_CALL_DEBUG__()              __TRACE_CALL_COMMON__()
    #define __TRACE_CALL_DEBUG_ARGS__(msg, ...) __TRACE_CALL_COMMON__()
  #else
    #define __TRACE_CALL_DEBUG__()
    #define __TRACE_CALL_DEBUG_ARGS__(msg, ...)
  #endif

  #define __TRACE_CALL_RESULT__(msg, ...)
  #define __TRACE_LINE__()
#endif  // !defined(DISABLE_DEBUG_TRACES) && !defined(DISABLE_TRACES)

// ---------------------------------------------------------------------------------
//                          HELPERS
#define SC2INT(v)               static_cast<int>(v)
#define BOOL2INT(v)             SC2INT(v)
#define BOOL2STR(v)             ((v) ? "true" : "false")

#define _DEF2STR_INTERNAL(s)    #s
#define DEF2STR(s)              _DEF2STR_INTERNAL(s)

#endif  // __HSMCPP_LOGGING_HPP__
