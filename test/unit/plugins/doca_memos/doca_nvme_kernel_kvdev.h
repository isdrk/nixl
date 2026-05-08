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
 * @file doca_nvme_kernel_kvdev.h
 * @page doca_nvme_kernel_kvdev
 * @defgroup DOCA_NVME_KERNEL_KVDEV DOCA NVMe Kernel KV Device
 * DOCA NVMe kernel backend for key-value operations.
 *
 * @{
 */
#ifndef DOCA_NVME_KERNEL_KVDEV_H_
#define DOCA_NVME_KERNEL_KVDEV_H_

#include <stdint.h>

#include <doca_compat.h>
#include <doca_error.h>

#include <doca_kvdev.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque structure representing a DOCA NVMe kernel KV Device.
 * The DOCA NVMe kernel KV device is an interface for a logical Key-Value backend that is backed by an NVMe namespace
 * managed by the kernel and exposed as a character device.
 * This structure extends struct doca_kvdev.
 * This structure is used by KV applications and services.
 */
struct doca_nvme_kernel_kvdev;

/*********************************************************************************************************************
 * DOCA NVMe Kernel KV Device Capabilities
 *********************************************************************************************************************/

/**
 * @brief Get the maximum length (in bytes, including null terminator) for the NVMe character device path.
 *
 * @details Use this cap to size buffers before calling doca_nvme_kernel_kvdev_set_path() or
 * doca_nvme_kernel_kvdev_get_path(). The path string may contain up to max_path_len - 1 characters plus the null
 * terminator.
 *
 * @param [out] max_path_len
 * The maximum length in bytes for the path string (including null terminator).
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'max_path_len' is NULL.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_nvme_kernel_kvdev_cap_get_max_path_len(uint32_t *max_path_len);

/*********************************************************************************************************************
 * DOCA NVMe Kernel KV Device API
 *********************************************************************************************************************/

/**
 * @brief Create a DOCA NVMe kernel KV device.
 *
 * @details The initialization flow for the KV device and its I/O contexts is as follows:
 * 1. Create a new KV device by calling doca_nvme_kernel_kvdev_create().
 * 2. Set the character device path for the underlying NVMe namespace by calling doca_nvme_kernel_kvdev_set_path()
 *    (e.g. "/dev/ng0n1").
 * 3. Set the NGUID of the underlying NVMe namespace by calling doca_kvdev_set_nguid().
 * 4. Call doca_kvdev_start() to finalize the initialization of the device and attach to the underlying NVMe namespace.
 *    Start will fail if the underlying NVMe namespace was not found or does not match the configured attributes.
 *    The device is now started and ready to use.
 * 5. Optionally query device capabilities using doca_kvdev_get_size(), doca_kvdev_get_max_key_len(),
 *    doca_kvdev_get_max_value_len(), and doca_kvdev_get_max_tasks() APIs.
 * 6. Create one or more I/O contexts by calling doca_nvme_kernel_kvdev_io_create(). It is recommended to create one I/O
 *    context per application I/O thread.
 * 7. Configure each I/O context:
 *    a. Set the number of tasks via doca_kvdev_io_set_num_tasks().
 *    b. Set the task completion callback via doca_kvdev_io_set_task_completion_cb().
 *    c. Set the task error callback via doca_kvdev_io_set_task_error_cb().
 * 8. Call doca_ctx_start() on each I/O context. The context moves to a "running" state.
 *
 * Once the I/O context is running, the application may allocate and submit tasks using
 * doca_kvdev_io_task_*_alloc_init(), and doca_task_submit() APIs.
 * Completions are delivered by calling doca_pe_progress().
 *
 * @param [out] kkvdev
 * The created DOCA NVMe kernel KV device.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kkvdev' is NULL.
 * - DOCA_ERROR_NO_MEMORY - failed to allocate memory.
 * @note Returned instance must be destroyed using doca_nvme_kernel_kvdev_destroy().
 */
DOCA_EXPERIMENTAL
doca_error_t doca_nvme_kernel_kvdev_create(struct doca_nvme_kernel_kvdev **kkvdev);

/**
 * @brief Destroy a DOCA NVMe kernel KV device.
 *
 * @details The teardown flow for the KV device and its I/O contexts is as follows:
 * 1. Stop each I/O context by calling doca_ctx_stop() on doca_kvdev_io_as_ctx(). Each I/O context
 *    moves to a "stopping" state and then to "idle" once all in-flight tasks have completed.
 * 2. Destroy each I/O context by calling doca_nvme_kernel_kvdev_io_destroy().
 * 3. Stop the KV device by calling doca_kvdev_stop(). doca_kvdev_stop() will fail if there are
 *    still associated I/O contexts.
 * 4. Release the KV device object by calling doca_nvme_kernel_kvdev_destroy().
 *
 * @param [in] kkvdev
 * The DOCA NVMe kernel KV device to destroy. Must be stopped and have no associated I/O contexts.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kkvdev' is NULL.
 * - DOCA_ERROR_BAD_STATE - If 'kkvdev' is started. Use doca_kvdev_stop() first.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_nvme_kernel_kvdev_destroy(struct doca_nvme_kernel_kvdev *kkvdev);

/**
 * @brief Convert DOCA NVMe kernel KV device instance into DOCA KV device.
 *
 * @param [in] kkvdev
 * DOCA NVMe kernel KV device instance. This must remain valid until after the DOCA KV device is no longer required.
 *
 * @return
 * DOCA KV device upon success, NULL otherwise.
 */
DOCA_EXPERIMENTAL
struct doca_kvdev *doca_nvme_kernel_kvdev_as_kvdev(struct doca_nvme_kernel_kvdev *kkvdev);

/**
 * @brief Set the NVMe character device path for the DOCA NVMe kernel KV device.
 *
 * @param [in] kkvdev
 * The DOCA NVMe kernel KV device to configure. Must not be started.
 * @param [in] path
 * Null-terminated path to the NVMe character device node (e.g. "/dev/ng0n1"). Length (including null terminator)
 * must not exceed the value returned by doca_nvme_kernel_kvdev_cap_get_max_path_len().
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kkvdev' or 'path' is NULL, or 'path' exceeds maximum length.
 * - DOCA_ERROR_BAD_STATE - If 'kkvdev' is already started. Use doca_kvdev_stop() to stop it.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_nvme_kernel_kvdev_set_path(struct doca_nvme_kernel_kvdev *kkvdev, const char *path);

/**
 * @brief Get the NVMe character device path of the DOCA NVMe kernel KV device.
 *
 * @details Copies the configured path into the user-allocated buffer. The implementation validates the buffer size
 * before copying to prevent overflow.
 *
 * @param [in] kkvdev
 * The DOCA NVMe kernel KV device to query.
 * @param [out] path
 * User-allocated buffer to receive the configured path (null-terminated).
 * @param [in] path_len
 * Length in bytes of the path buffer (including null terminator).
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kkvdev' or 'path' is NULL, or 'path_len' is smaller than the configured path
 *   length for this device.
 * - DOCA_ERROR_NOT_FOUND - If a path was not configured for this device.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_nvme_kernel_kvdev_get_path(const struct doca_nvme_kernel_kvdev *kkvdev,
					     char *path,
					     uint32_t path_len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DOCA_NVME_KERNEL_KVDEV_H_ */

/** @} */
