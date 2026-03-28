#ifndef MS_COMMON_H
#define MS_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MS_STACK_MAX           256
#define MS_FRAMES_MAX          64
#define MS_GC_HEAP_GROW_FACTOR 2
#define MS_MAX_LOCALS          256
#define MS_MAX_UPVALUES        256
#define MS_TABLE_MAX_LOAD      0.75

typedef enum { MS_OK, MS_COMPILE_ERROR, MS_RUNTIME_ERROR } MsResult;

#ifdef MS_DEBUG_LOG_GC
  #define MS_DEBUG_LOG_GC_EXECUTE(code) code
#else
  #define MS_DEBUG_LOG_GC_EXECUTE(code)
#endif

#ifdef MS_DEBUG_STRESS_GC
  #define MS_DEBUG_STRESS_GC_EXECUTE(code) code
#else
  #define MS_DEBUG_STRESS_GC_EXECUTE(code)
#endif

#ifdef MS_DEBUG_TRACE_EXECUTION
  #define MS_DEBUG_TRACE_EXECUTE(code) code
#else
  #define MS_DEBUG_TRACE_EXECUTE(code)
#endif

#endif
