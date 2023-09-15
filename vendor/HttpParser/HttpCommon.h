#ifndef _http11_common_h
#define _http11_common_h

#include <sys/types.h>
#include <cerrno>
#define check(A, M, ...) if(!(A)) { errno=0; goto error; }
typedef void (*element_cb)(void *data, const char *at, size_t length);
typedef void (*field_cb)(void *data, const char *field, size_t flen, const char *value, size_t vlen);

#endif