/*
 * DOCA KV Mock - Progress Engine API
 * Mock implementation of DOCA progress engine
 */

#ifndef DOCA_PE_H_
#define DOCA_PE_H_

#include <stdint.h>
#include <stdlib.h>

#include <doca_compat.h>
#include <doca_error.h>
#include <doca_types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct doca_ctx;
struct doca_pe;
struct doca_task;

/**
 * @brief Function to execute on task completion.
 */
typedef void (*doca_task_completion_cb_t)(struct doca_task *task,
					  union doca_data task_user_data,
					  union doca_data ctx_user_data);

/**
 * @brief Creates DOCA progress engine
 */
doca_error_t doca_pe_create(struct doca_pe **pe);

/**
 * @brief Destroy DOCA progress engine
 */
doca_error_t doca_pe_destroy(struct doca_pe *pe);

/**
 * @brief Connect context to a progress engine
 */
doca_error_t doca_pe_connect_ctx(struct doca_pe *pe, struct doca_ctx *ctx);

/**
 * @brief Progress one or more tasks
 * @return Number of events processed
 */
int doca_pe_progress(struct doca_pe *pe);

/**
 * @brief Get notification handle for event-driven operation
 */
doca_error_t doca_pe_get_notification_handle(struct doca_pe *pe, doca_notification_handle_t *handle);

/**
 * @brief Request notification when events are available
 */
doca_error_t doca_pe_request_notification(struct doca_pe *pe);

/**
 * @brief Clear notification after event
 */
void doca_pe_clear_notification(struct doca_pe *pe, doca_notification_handle_t handle);

/**
 * @brief Submit a DOCA task
 */
doca_error_t doca_task_submit(struct doca_task *task);

/**
 * @brief Free a DOCA task
 */
void doca_task_free(struct doca_task *task);

/**
 * @brief Get user data from task
 */
doca_error_t doca_task_get_user_data(const struct doca_task *task, union doca_data *user_data);

/**
 * @brief Set user data to task
 */
doca_error_t doca_task_set_user_data(struct doca_task *task, union doca_data user_data);

/**
 * @brief Get the status/result of a completed task.
 *
 * Valid only after the task's completion or error callback has been invoked.
 */
doca_error_t doca_task_get_status(const struct doca_task *task);

#ifdef __cplusplus
}
#endif

#endif /* DOCA_PE_H_ */
