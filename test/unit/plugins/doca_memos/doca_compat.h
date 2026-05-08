/*
 * DOCA KV Mock - Compatibility Definitions
 * Mock implementation of DOCA compatibility macros
 */

#ifndef DOCA_COMPAT_H_
#define DOCA_COMPAT_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__linux__)

#define DOCA_USED __attribute__((used))
#define DOCA_STABLE __attribute__((visibility("default"))) DOCA_USED

#ifndef DOCA_ALLOW_EXPERIMENTAL_API
#define DOCA_EXPERIMENTAL \
	__attribute__((deprecated("Symbol is defined as experimental"), section(".text.experimental"))) DOCA_STABLE
#else
#define DOCA_EXPERIMENTAL __attribute__((section(".text.experimental"))) DOCA_STABLE
#endif

#else /* Windows or other */

#define __attribute__(_x_)
#define DOCA_STABLE
#define DOCA_EXPERIMENTAL

#endif

/* Compiler optimization hints */
#define doca_likely(x)   __builtin_expect(!!(x), 1)
#define doca_unlikely(x) __builtin_expect(!!(x), 0)

#ifdef __cplusplus
}
#endif

#endif /* DOCA_COMPAT_H_ */
