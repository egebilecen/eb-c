#if !defined(TRACK_H)
#define TRACK_H

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */
#include <stdlib.h>

/* -------------------------------------------------------------------------- */
/*                                   Macros                                   */
/* -------------------------------------------------------------------------- */
#define TRACKED_VAR_NAME(func) __tracked_##func##_counter

#define TRACKED_FUNC_DECL(ret_type, func, args_decl, args_call) \
    size_t TRACKED_VAR_NAME(func) = 0;                          \
    ret_type __tracked_##func args_decl;

#define TRACKED_FUNC_IMPL(ret_type, func, args_decl, args_call) \
    inline ret_type __tracked_##func args_decl                  \
    {                                                           \
        TRACKED_VAR_NAME(func)                                  \
        ++;                                                     \
        return func args_call;                                  \
    }

#define TRACKED_FUNC_IMPL_NO_RETURN(ret_type, func, args_decl, args_call) \
    inline ret_type __tracked_##func args_decl                            \
    {                                                                     \
        TRACKED_VAR_NAME(func)                                            \
        ++;                                                               \
        func args_call;                                                   \
    }

#ifdef ENABLE_FUNCTION_TRACKING
#define TRACKED_FUNC_DEFINE(ret_type, func, args_decl, args_call) \
    TRACKED_FUNC_DECL(ret_type, func, args_decl, args_call)       \
    TRACKED_FUNC_IMPL(ret_type, func, args_decl, args_call)

#define TRACKED_FUNC_DEFINE_NO_RETURN(ret_type, func, args_decl, args_call) \
    TRACKED_FUNC_DECL(ret_type, func, args_decl, args_call)                 \
    TRACKED_FUNC_IMPL_NO_RETURN(ret_type, func, args_decl, args_call)

#define TRACKED_FUNC_CALL(func, args_call) \
    __tracked_##func args_call

#define TRACKED_FUNC_RESET_COUNTER(func) \
    TRACKED_VAR_NAME(func) = 0

#define TRACKED_FUNC_GET_COUNTER(func) \
    TRACKED_VAR_NAME(func)

#define TRACKED_FUNC_INCREMENT_COUNTER(func) \
    TRACKED_VAR_NAME(func)                   \
    ++

#define TRACKED_FUNC_COMPARE_TWO_COUNTERS(func1, func2) \
    assert((TRACKED_VAR_NAME(func1) == TRACKED_VAR_NAME(func2)))
#else
#define TRACKED_FUNC_DEFINE(ret_type, func, args_decl, args_call) \
    ret_type func args_decl;
#define TRACKED_FUNC_DEFINE_NO_RETURN(ret_type, func, args_decl, args_call) \
    ret_type func args_decl;
#define TRACKED_FUNC_CALL(func, args_call) func args_call
#define TRACKED_FUNC_RESET_COUNTER(func) ((void)0)
#define TRACKED_FUNC_GET_COUNTER(func) (0)
#define TRACKED_FUNC_INCREMENT_COUNTER(func) ((void)0)
#define TRACKED_FUNC_COMPARE_TWO_COUNTERS(func1, func2) ((void)0)
#endif // ENABLE_FUNCTION_TRACKING

/* -------------------------------------------------------------------------- */
/*                              Tracked Functions                             */
/* -------------------------------------------------------------------------- */
TRACKED_FUNC_DEFINE(void *, malloc, (size_t size), (size))
TRACKED_FUNC_DEFINE(void *, calloc, (size_t num, size_t size), (num, size))
TRACKED_FUNC_DEFINE_NO_RETURN(void, free, (void *ptr), (ptr))

#endif // TRACK_H
