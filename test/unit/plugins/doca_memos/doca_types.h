/*
 * DOCA KV Mock - Common Types
 * Mock implementation of DOCA common types
 */

#ifndef DOCA_TYPES_H_
#define DOCA_TYPES_H_

#include <stdint.h>

#ifdef __linux__
#include <linux/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __linux__
/** 'fd' for blocking with epoll/select/poll, event type will be "read ready" */
typedef int doca_notification_handle_t;
#define doca_event_invalid_handle -1
#else
typedef void *doca_notification_handle_t;
#define doca_event_invalid_handle NULL
#endif

/**
 * @brief Convenience type for representing opaque data
 */
union doca_data {
	void *ptr;
	uint64_t u64;
};

/**
 * @brief Struct to represent a gather list
 */
struct doca_gather_list {
	void *addr;
	uint64_t len;
	struct doca_gather_list *next;
};

#ifdef __cplusplus
}
#endif

#endif /* DOCA_TYPES_H_ */
