#ifndef __SERVER_COMMON_H__
#define __SERVER_COMMON_H__
#include <assert.h>

#if defined __GUNC__ ||defined  __llvm__
# define LICKLY(x) __builtin_expect(!!(x), 1)
# define UNLICKLY(x) __builtin_expect(!!(x), 0)
#else
#  define LICKLY(x) (x) 
#  define UNLICKLY(x) (x)
#endif



#define ASSERT(x, ...) assert((x) && (__VA_ARGS__))

#define ENSURE(x, message) \
    if(!(x)) { \
        throw std::logic_error(message);\
    }

#endif