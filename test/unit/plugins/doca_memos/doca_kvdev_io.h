/*
 * Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES, ALL RIGHTS RESERVED.
 *
 * This software product is a proprietary product of NVIDIA CORPORATION &
 * AFFILIATES (the "Company") and all right, title, and interest in and to the
 * software product, including all associated intellectual property rights, are
 * and shall remain exclusively with the Company.
 *
 * This software product is governed by the End User License Agreement
 * provided with the software product.
 *
 */

/**
 * @file doca_kvdev_io.h
 * @page doca_kvdev_io
 * @defgroup DOCA_KVDEV_IO DOCA KV Device I/O Context
 * DOCA KV I/O context and task API for key-value operations on NVMe KV devices.
 * For more details please refer to the user guide on DOCA devzone.
 *
 * @{
 */
#ifndef DOCA_KVDEV_IO_H_
#define DOCA_KVDEV_IO_H_

#include <stdint.h>
#include <sys/uio.h>

#include <doca_compat.h>
#include <doca_ctx.h>
#include <doca_error.h>
#include <doca_pe.h>
#include <doca_types.h>

#include <doca_kvdev.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque structure representing a DOCA KV I/O context.
 * This structure extends struct doca_ctx. It is thread-unsafe and is typically bound to a core.
 * This structure is used by KV applications and services.
 */
struct doca_kvdev_io;

/**
 * @brief Opaque structure representing a DOCA KV store task.
 * This structure is used by KV applications and services.
 */
struct doca_kvdev_io_task_store;

/**
 * @brief Opaque structure representing a DOCA KV retrieve task.
 * This structure is used by KV applications and services.
 */
struct doca_kvdev_io_task_retrieve;

/**
 * @brief Opaque structure representing a DOCA KV exist task.
 * This structure is used by KV applications and services.
 */
struct doca_kvdev_io_task_exist;

/*********************************************************************************************************************
 * DOCA KV I/O Context API
 *********************************************************************************************************************/

/**
 * @brief Convert DOCA KV I/O context instance into DOCA context.
 *
 * @param [in] io
 * DOCA KV I/O context instance. This must remain valid until after the DOCA context is no longer required.
 *
 * @return
 * doca ctx upon success, NULL otherwise.
 */
DOCA_EXPERIMENTAL
struct doca_ctx *doca_kvdev_io_as_ctx(struct doca_kvdev_io *io);

/**
 * @brief Get the number of tasks the DOCA KV I/O context can allocate.
 *
 * @param [in] io
 * The DOCA KV I/O context instance.
 * @param [out] num_tasks
 * Number of tasks the DOCA KV I/O context can allocate.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'io' or 'num_tasks' is NULL.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_io_get_num_tasks(const struct doca_kvdev_io *io, uint32_t *num_tasks);

/**
 * @brief Set the number of tasks the DOCA KV I/O context can allocate.
 *
 * @details The I/O context must be idle. If called multiple times while the I/O context is idle, only the last value
 * set before the I/O context is started applies. The number of tasks must not exceed the value returned by
 * doca_kvdev_get_max_tasks() for the KV device associated with the I/O context, otherwise the I/O context start will
 * fail.
 *
 * @param [in] io
 * The DOCA KV I/O context instance. Must be idle.
 * @param [in] num_tasks
 * Number of tasks the DOCA KV I/O context can allocate.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'io' is NULL or 'num_tasks' is zero.
 * - DOCA_ERROR_BAD_STATE - If 'io' is not idle.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_io_set_num_tasks(struct doca_kvdev_io *io, uint32_t num_tasks);

/**
 * @brief Callback function type invoked on DOCA KV I/O context task completion.
 *
 * @details This function is called when the associated task has completed.
 * When this function is invoked, ownership of the task object is transferred from the context back to the user.
 *
 * Inside this callback, the user must not call doca_pe_progress().
 * For more information, please see doca_pe_progress().
 *
 * @param [in] task
 * The completed DOCA task.
 * @param [in] task_user_data
 * The user data associated with the task.
 * @param [in] ctx_user_data
 * The user data associated with the corresponding DOCA KV I/O context.
 */
typedef void (*doca_kvdev_io_task_completion_cb_t)(struct doca_task *task,
						   union doca_data task_user_data,
						   union doca_data ctx_user_data);

/**
 * @brief Set the DOCA KV I/O context task completion callback.
 *
 * @details The I/O context must be idle. If called multiple times while the I/O context is idle, only the last value
 * set before the I/O context is started applies.
 *
 * @param [in] io
 * The DOCA KV I/O context instance. Must be idle.
 * @param [in] task_completion_cb
 * Callback for successful task completion.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'io' or 'task_completion_cb' is NULL.
 * - DOCA_ERROR_BAD_STATE - If 'io' is not idle.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_io_set_task_completion_cb(struct doca_kvdev_io *io,
						  doca_kvdev_io_task_completion_cb_t task_completion_cb);

/**
 * @brief Set the DOCA KV I/O context task error callback.
 *
 * @details The I/O context must be idle. If called multiple times while the I/O context is idle, only the last value
 * set before the I/O context is started applies.
 *
 * @param [in] io
 * The DOCA KV I/O context instance. Must be idle.
 * @param [in] task_error_cb
 * Callback for task error completion.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'io' or 'task_error_cb' is NULL.
 * - DOCA_ERROR_BAD_STATE - If 'io' is not idle.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_io_set_task_error_cb(struct doca_kvdev_io *io,
					     doca_kvdev_io_task_completion_cb_t task_error_cb);

/*********************************************************************************************************************
 * DOCA KV Store Task API
 *********************************************************************************************************************/

/**
 * @brief Allocate and initialize a DOCA KV store task.
 *
 * @details Task completion error codes (delivered via error callback):
 * - DOCA_ERROR_INVALID_VALUE - Invalid field in command.
 * - DOCA_ERROR_NO_MEMORY - Capacity of the device exceeded.
 * - DOCA_ERROR_BAD_STATE - Device backend is not ready.
 * - DOCA_ERROR_IO_FAILED - I/O operation failed due to media error.
 * - DOCA_ERROR_NOT_FOUND - Key does not exist (only when "must exist" option is set).
 * - DOCA_ERROR_ALREADY_EXIST - Key already exists (only when "do not overwrite" option is set).
 *
 * @param [in] io
 * DOCA KV I/O context from which to allocate the task. Must be in the "running" state.
 * @param [in] task_user_data
 * User data to be associated with the task.
 * @param [out] task
 * The allocated and initialized DOCA KV store task.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'io' or 'task' is NULL.
 * - DOCA_ERROR_NO_MEMORY - Allocation failure.
 * @note Configure the key and value using doca_kvdev_io_task_store_set_key_value_conf() before submitting the task.
 * Optionally, set the "do not overwrite" using doca_kvdev_io_task_store_set_do_not_overwrite() or "must exist"
 * using doca_kvdev_io_task_store_set_must_exist() before submitting the task.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_io_task_store_alloc_init(struct doca_kvdev_io *io,
						 union doca_data task_user_data,
						 struct doca_kvdev_io_task_store **task);

/**
 * @brief Configure the key and value for a DOCA KV store task.
 *
 * @param [in] task
 * The store task.
 * @param [in] key
 * Pointer to the key buffer.
 * @param [in] key_len
 * Length of the key in bytes. Must not exceed the value returned by doca_kvdev_get_max_key_len() for the KV device
 * associated with the task.
 * @param [in] value_iovecs
 * Array of value I/O vectors.
 * @param [in] iovcnt
 * Number of elements in value_iovecs.
 * @param [in] value_len
 * Total length in bytes of the value to store. Must not exceed the value returned by doca_kvdev_get_max_value_len()
 * for the KV device associated with the task.
 * Must match the aggregate of the value laid out in value_iovecs.
 * @note Must only be called when the task is in the ownership of the user. Otherwise, behavior is undefined.
 */
DOCA_EXPERIMENTAL
void doca_kvdev_io_task_store_set_key_value_conf(struct doca_kvdev_io_task_store *task,
						 const void *key,
						 uint16_t key_len,
						 struct iovec *value_iovecs,
						 uint32_t iovcnt,
						 uint32_t value_len);

/**
 * @brief Get the key pointer from a DOCA KV store task.
 *
 * @param [in] task
 * The store task.
 *
 * @return
 * Key buffer pointer set by doca_kvdev_io_task_store_set_key_value_conf(), or NULL if unset.
 */
DOCA_EXPERIMENTAL
const void *doca_kvdev_io_task_store_get_key(const struct doca_kvdev_io_task_store *task);

/**
 * @brief Get the key length from a DOCA KV store task.
 *
 * @param [in] task
 * The store task.
 *
 * @return
 * Key length in bytes set by doca_kvdev_io_task_store_set_key_value_conf(), or zero if unset.
 */
DOCA_EXPERIMENTAL
uint16_t doca_kvdev_io_task_store_get_key_len(const struct doca_kvdev_io_task_store *task);

/**
 * @brief Get the value I/O vectors from a DOCA KV store task.
 *
 * @param [in] task
 * The store task.
 *
 * @return
 * Value iovec array set by doca_kvdev_io_task_store_set_key_value_conf(), or NULL if unset.
 */
DOCA_EXPERIMENTAL
const struct iovec *doca_kvdev_io_task_store_get_value_iovecs(const struct doca_kvdev_io_task_store *task);

/**
 * @brief Get the number of value I/O vectors from a DOCA KV store task.
 *
 * @param [in] task
 * The store task.
 *
 * @return
 * Iovec count set by doca_kvdev_io_task_store_set_key_value_conf(), or zero if unset.
 */
DOCA_EXPERIMENTAL
uint32_t doca_kvdev_io_task_store_get_value_iovcnt(const struct doca_kvdev_io_task_store *task);

/**
 * @brief Get the value length from a DOCA KV store task.
 *
 * @param [in] task
 * The store task.
 *
 * @return
 * Value length in bytes set by doca_kvdev_io_task_store_set_key_value_conf(), or zero if unset.
 */
DOCA_EXPERIMENTAL
uint32_t doca_kvdev_io_task_store_get_value_len(const struct doca_kvdev_io_task_store *task);

/**
 * @brief Set the "do not overwrite" option for a DOCA KV store task.
 *
 * @details When set to 1, the store operation aborts with DOCA_ERROR_ALREADY_EXIST if the
 * specified key already exists in the device. Mutually exclusive with the "must exist"
 * option — both should not be set to 1 simultaneously.
 *
 * @param [in] task
 * The store task.
 * @param [in] do_not_overwrite
 * Set to 1 to enable, 0 to disable (default).
 * @note Must only be called when the task is in the ownership of the user. Otherwise, behavior is undefined.
 */
DOCA_EXPERIMENTAL
void doca_kvdev_io_task_store_set_do_not_overwrite(struct doca_kvdev_io_task_store *task, uint8_t do_not_overwrite);

/**
 * @brief Get the "do not overwrite" option from a DOCA KV store task.
 *
 * @param [in] task
 * The store task.
 *
 * @return
 * 1 if the "do not overwrite" option is set, 0 otherwise.
 * @note Must only be called when the task is in the ownership of the user. Otherwise, behavior is undefined.
 */
DOCA_EXPERIMENTAL
uint8_t doca_kvdev_io_task_store_get_do_not_overwrite(const struct doca_kvdev_io_task_store *task);

/**
 * @brief Set the "must exist" option for a DOCA KV store task.
 *
 * @details When set to 1, the store operation aborts with DOCA_ERROR_NOT_FOUND if the
 * specified key does not already exist in the device. Mutually exclusive with the
 * "do not overwrite" option — both should not be set to 1 simultaneously.
 *
 * @param [in] task
 * The store task.
 * @param [in] must_exist
 * Set to 1 to enable, 0 to disable (default).
 * @note Must only be called when the task is in the ownership of the user. Otherwise, behavior is undefined.
 */
DOCA_EXPERIMENTAL
void doca_kvdev_io_task_store_set_must_exist(struct doca_kvdev_io_task_store *task, uint8_t must_exist);

/**
 * @brief Get the "must exist" option from a DOCA KV store task.
 *
 * @param [in] task
 * The store task.
 *
 * @return
 * 1 if the "must exist" option is set, 0 otherwise.
 * @note Must only be called when the task is in the ownership of the user. Otherwise, behavior is undefined.
 */
DOCA_EXPERIMENTAL
uint8_t doca_kvdev_io_task_store_get_must_exist(const struct doca_kvdev_io_task_store *task);

/**
 * @brief Convert a DOCA KV store task to a DOCA task.
 *
 * @param [in] task
 * The DOCA KV store task. Must be in the ownership of the user.
 *
 * @return
 * doca_task on success, NULL otherwise.
 */
DOCA_EXPERIMENTAL
struct doca_task *doca_kvdev_io_task_store_as_task(struct doca_kvdev_io_task_store *task);

/**
 * @brief Convert a DOCA task to a DOCA KV store task.
 *
 * @param [in] task
 * The doca_task. Must be a store task and in the ownership of the user.
 *
 * @return
 * DOCA KV store task on success, NULL otherwise.
 */
DOCA_EXPERIMENTAL
struct doca_kvdev_io_task_store *doca_kvdev_io_task_store_from_task(struct doca_task *task);

/*********************************************************************************************************************
 * DOCA KV Retrieve Task API
 *********************************************************************************************************************/

/**
 * @brief Allocate and initialize a DOCA KV retrieve task.
 *
 * @details Task completion error codes (delivered via error callback):
 * - DOCA_ERROR_INVALID_VALUE - Invalid field in command.
 * - DOCA_ERROR_BAD_STATE - Device backend is not ready.
 * - DOCA_ERROR_IO_FAILED - I/O operation failed due to media error.
 * - DOCA_ERROR_NOT_FOUND - The specified key does not exist in the device.
 *
 * @param [in] io
 * DOCA KV I/O context from which to allocate the task. Must be in the "running" state.
 * @param [in] task_user_data
 * User data to be associated with the task.
 * @param [out] task
 * The allocated and initialized DOCA KV retrieve task.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'io' or 'task' is NULL.
 * - DOCA_ERROR_NO_MEMORY - Allocation failure.
 * @note Configure the key and value buffer using doca_kvdev_io_task_retrieve_set_key_value_conf() before submitting
 * the task.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_io_task_retrieve_alloc_init(struct doca_kvdev_io *io,
						    union doca_data task_user_data,
						    struct doca_kvdev_io_task_retrieve **task);

/**
 * @brief Configure the key and value buffer for a DOCA KV retrieve task.
 *
 * @details The aggregate byte capacity described by 'value_iovecs' is submitted to the
 * device as the Host Buffer Size (HBS). If the stored value is larger than HBS, the
 * device writes only the first HBS bytes into the provided buffers and returns the full
 * stored value length at completion. To detect truncation, compare 'value_len' against the
 * result returned by doca_kvdev_io_task_retrieve_get_result_value_len() after completion,
 * and reissue with a larger buffer if needed.
 *
 * @param [in] task
 * The retrieve task.
 * @param [in] key
 * Pointer to the key buffer.
 * @param [in] key_len
 * Length of the key in bytes. Must not exceed the value returned by doca_kvdev_get_max_key_len() for the KV device
 * associated with the task.
 * @param [in] value_iovecs
 * Array of value I/O vectors describing receive buffers.
 * @param [in] iovcnt
 * Number of elements in value_iovecs.
 * @param [in] value_len
 * Total capacity in bytes for the receive buffers described by value_iovecs. Must not exceed the value returned by
 * doca_kvdev_get_max_value_len() for the KV device associated with the task.
 * Must match the aggregate receive capacity in value_iovecs.
 * @note Must only be called when the task is in the ownership of the user. Otherwise, behavior is undefined.
 */
DOCA_EXPERIMENTAL
void doca_kvdev_io_task_retrieve_set_key_value_conf(struct doca_kvdev_io_task_retrieve *task,
						    const void *key,
						    uint16_t key_len,
						    struct iovec *value_iovecs,
						    uint32_t iovcnt,
						    uint32_t value_len);

/**
 * @brief Get the key pointer from a DOCA KV retrieve task.
 *
 * @param [in] task
 * The retrieve task.
 *
 * @return
 * Key buffer pointer set by doca_kvdev_io_task_retrieve_set_key_value_conf(), or NULL if unset.
 */
DOCA_EXPERIMENTAL
const void *doca_kvdev_io_task_retrieve_get_key(const struct doca_kvdev_io_task_retrieve *task);

/**
 * @brief Get the key length from a DOCA KV retrieve task.
 *
 * @param [in] task
 * The retrieve task.
 *
 * @return
 * Key length in bytes set by doca_kvdev_io_task_retrieve_set_key_value_conf(), or zero if unset.
 */
DOCA_EXPERIMENTAL
uint16_t doca_kvdev_io_task_retrieve_get_key_len(const struct doca_kvdev_io_task_retrieve *task);

/**
 * @brief Get the value I/O vectors from a DOCA KV retrieve task.
 *
 * @param [in] task
 * The retrieve task.
 *
 * @return
 * Value iovec array set by doca_kvdev_io_task_retrieve_set_key_value_conf(), or NULL if unset.
 */
DOCA_EXPERIMENTAL
const struct iovec *doca_kvdev_io_task_retrieve_get_value_iovecs(const struct doca_kvdev_io_task_retrieve *task);

/**
 * @brief Get the number of value I/O vectors from a DOCA KV retrieve task.
 *
 * @param [in] task
 * The retrieve task.
 *
 * @return
 * Iovec count set by doca_kvdev_io_task_retrieve_set_key_value_conf(), or zero if unset.
 */
DOCA_EXPERIMENTAL
uint32_t doca_kvdev_io_task_retrieve_get_value_iovcnt(const struct doca_kvdev_io_task_retrieve *task);

/**
 * @brief Get the value buffer capacity length from a DOCA KV retrieve task.
 *
 * @param [in] task
 * The retrieve task.
 *
 * @return
 * Value length in bytes set by doca_kvdev_io_task_retrieve_set_key_value_conf(), or zero if unset.
 */
DOCA_EXPERIMENTAL
uint32_t doca_kvdev_io_task_retrieve_get_value_len(const struct doca_kvdev_io_task_retrieve *task);

/**
 * @brief Get the actual value length returned by the device for a completed DOCA KV retrieve task.
 *
 * @details Returns the full stored value length as reported by the device in the completion.
 * If this exceeds the 'value_len' configured via doca_kvdev_io_task_retrieve_set_key_value_conf(),
 * the data was truncated and the caller should reissue with a larger buffer.
 * Valid only after a successful task completion callback has fired and returned task ownership to the user.
 *
 * @param [in] task
 * The retrieve task.
 *
 * @return
 * Actual value length in bytes as reported by the device.
 */
DOCA_EXPERIMENTAL
uint32_t doca_kvdev_io_task_retrieve_get_result_value_len(const struct doca_kvdev_io_task_retrieve *task);

/**
 * @brief Convert a DOCA KV retrieve task to a DOCA task.
 *
 * @param [in] task
 * The DOCA KV retrieve task. Must be in the ownership of the user.
 *
 * @return
 * doca_task on success, NULL otherwise.
 */
DOCA_EXPERIMENTAL
struct doca_task *doca_kvdev_io_task_retrieve_as_task(struct doca_kvdev_io_task_retrieve *task);

/**
 * @brief Convert a DOCA task to a DOCA KV retrieve task.
 *
 * @param [in] task
 * The doca_task. Must be a retrieve task and in the ownership of the user.
 *
 * @return
 * DOCA KV retrieve task on success, NULL otherwise.
 */
DOCA_EXPERIMENTAL
struct doca_kvdev_io_task_retrieve *doca_kvdev_io_task_retrieve_from_task(struct doca_task *task);

/*********************************************************************************************************************
 * DOCA KV Exist Task API
 *********************************************************************************************************************/

/**
 * @brief Allocate and initialize a DOCA KV exist task.
 *
 * @details Task completion error codes (delivered via error callback):
 * - DOCA_ERROR_INVALID_VALUE - Invalid field in command.
 * - DOCA_ERROR_BAD_STATE - Device backend is not ready.
 * - DOCA_ERROR_IO_FAILED - I/O operation failed due to media error.
 * - DOCA_ERROR_NOT_FOUND - The specified key does not exist in the device.
 *
 * @param [in] io
 * DOCA KV I/O context from which to allocate the task. Must be in the "running" state.
 * @param [in] task_user_data
 * User data to be associated with the task.
 * @param [out] task
 * The allocated and initialized DOCA KV exist task.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'io' or 'task' is NULL.
 * - DOCA_ERROR_NO_MEMORY - Allocation failure.
 * @note Configure the key using doca_kvdev_io_task_exist_set_key_conf() before submitting the task.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_io_task_exist_alloc_init(struct doca_kvdev_io *io,
						 union doca_data task_user_data,
						 struct doca_kvdev_io_task_exist **task);

/**
 * @brief Configure the key for a DOCA KV exist task.
 *
 * @param [in] task
 * The exist task.
 * @param [in] key
 * Pointer to the key buffer.
 * @param [in] key_len
 * Length of the key in bytes. Must not exceed the value returned by doca_kvdev_get_max_key_len() for the KV device
 * associated with the task.
 * @note Must only be called when the task is in the ownership of the user. Otherwise, behavior is undefined.
 */
DOCA_EXPERIMENTAL
void doca_kvdev_io_task_exist_set_key_conf(struct doca_kvdev_io_task_exist *task, const void *key, uint16_t key_len);

/**
 * @brief Get the key pointer from a DOCA KV exist task.
 *
 * @param [in] task
 * The exist task.
 *
 * @return
 * Key buffer pointer set by doca_kvdev_io_task_exist_set_key_conf(), or NULL if unset.
 */
DOCA_EXPERIMENTAL
const void *doca_kvdev_io_task_exist_get_key(const struct doca_kvdev_io_task_exist *task);

/**
 * @brief Get the key length from a DOCA KV exist task.
 *
 * @param [in] task
 * The exist task.
 *
 * @return
 * Key length in bytes set by doca_kvdev_io_task_exist_set_key_conf(), or zero if unset.
 */
DOCA_EXPERIMENTAL
uint16_t doca_kvdev_io_task_exist_get_key_len(const struct doca_kvdev_io_task_exist *task);

/**
 * @brief Convert a DOCA KV exist task to a DOCA task.
 *
 * @param [in] task
 * The DOCA KV exist task. Must be in the ownership of the user.
 *
 * @return
 * doca_task on success, NULL otherwise.
 */
DOCA_EXPERIMENTAL
struct doca_task *doca_kvdev_io_task_exist_as_task(struct doca_kvdev_io_task_exist *task);

/**
 * @brief Convert a DOCA task to a DOCA KV exist task.
 *
 * @param [in] task
 * The doca_task. Must be an exist task and in the ownership of the user.
 *
 * @return
 * DOCA KV exist task on success, NULL otherwise.
 */
DOCA_EXPERIMENTAL
struct doca_kvdev_io_task_exist *doca_kvdev_io_task_exist_from_task(struct doca_task *task);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DOCA_KVDEV_IO_H_ */

/** @} */
