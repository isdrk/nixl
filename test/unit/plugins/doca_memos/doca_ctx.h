/*
 * DOCA KV Mock - Context API
 * Mock implementation of DOCA context
 */

#ifndef DOCA_CTX_H_
#define DOCA_CTX_H_

#include <stddef.h>
#include <stdint.h>

#include <doca_compat.h>
#include <doca_error.h>
#include <doca_types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct doca_ctx;
struct doca_task;

/**
 * @brief Context states
 */
enum doca_ctx_states {
	DOCA_CTX_STATE_IDLE = 0,
	DOCA_CTX_STATE_STARTING = 1,
	DOCA_CTX_STATE_RUNNING = 2,
	DOCA_CTX_STATE_STOPPING = 3,
};

/**
 * @brief Finalizes all configurations, and starts the DOCA CTX.
 */
DOCA_STABLE
doca_error_t doca_ctx_start(struct doca_ctx *ctx);

/**
 * @brief Stops the context allowing reconfiguration.
 */
DOCA_STABLE
doca_error_t doca_ctx_stop(struct doca_ctx *ctx);

/**
 * @brief Get number of in flight tasks in a doca context
 */
DOCA_STABLE
doca_error_t doca_ctx_get_num_inflight_tasks(const struct doca_ctx *ctx, size_t *num_inflight_tasks);

/**
 * @brief set user data to context
 */
DOCA_STABLE
doca_error_t doca_ctx_set_user_data(struct doca_ctx *ctx, union doca_data user_data);

/**
 * @brief get user data from context
 */
DOCA_STABLE
doca_error_t doca_ctx_get_user_data(const struct doca_ctx *ctx, union doca_data *user_data);

/**
 * @brief Get context state
 */
DOCA_STABLE
doca_error_t doca_ctx_get_state(const struct doca_ctx *ctx, enum doca_ctx_states *state);

#ifdef __cplusplus
}
#endif

#endif /* DOCA_CTX_H_ */
