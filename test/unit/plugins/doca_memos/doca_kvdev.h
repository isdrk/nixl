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
 * @file doca_kvdev.h
 * @page doca_kvdev
 * @defgroup DOCA_KVDEV DOCA KV Device
 * DOCA KV device API for key-value operations on NVMe KV devices.
 * For more details please refer to the user guide on DOCA devzone.
 *
 * @{
 */
#ifndef DOCA_KVDEV_H_
#define DOCA_KVDEV_H_

#include <stdint.h>

#include <doca_compat.h>
#include <doca_error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Length in bytes of an NVMe namespace NGUID for DOCA KV device APIs, as defined by NVMe specification.
 */
#define DOCA_KVDEV_NGUID_LEN 16

/**
 * @brief Opaque structure representing a DOCA KV Device instance.
 * The DOCA KV device is an abstract interface for a logical Key-Value backend.
 * Access to this backend is provided through the DOCA KV Device I/O contexts.
 * This structure is used by KV applications and services.
 */
struct doca_kvdev;

/*********************************************************************************************************************
 * DOCA KV Device API
 *********************************************************************************************************************/

/**
 * @brief Set the namespace GUID for the DOCA KV device.
 *
 * @details Must be called before doca_kvdev_start(). Upon start, the NGUID reported by the underlying
 * device is compared against the configured NGUID. If they differ, doca_kvdev_start() fails with DOCA_ERROR_NOT_FOUND.
 *
 * @param [in] kvdev
 * The DOCA KV device to configure. Must not be started.
 * @param [in] nguid
 * NGUID of the namespace. The 'nguid' must point to a buffer of exactly DOCA_KVDEV_NGUID_LEN bytes.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kvdev' or 'nguid' is NULL.
 * - DOCA_ERROR_BAD_STATE - If 'kvdev' is already started. Use doca_kvdev_stop() to stop it.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_set_nguid(struct doca_kvdev *kvdev, const uint8_t *nguid);

/**
 * @brief Copy the configured NGUID from the DOCA KV device into the caller-supplied buffer.
 *
 * @param [in] kvdev
 * The DOCA KV device to query.
 * @param [out] nguid
 * Buffer to receive the NGUID.
 * @param [in] nguid_len
 * Size of 'nguid' in bytes. Must be at least DOCA_KVDEV_NGUID_LEN.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kvdev' or 'nguid' is NULL, or 'nguid_len' is smaller than DOCA_KVDEV_NGUID_LEN.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_get_nguid(const struct doca_kvdev *kvdev, uint8_t *nguid, uint16_t nguid_len);

/**
 * @brief Start a DOCA KV device.
 *
 * @param [in] kvdev
 * The DOCA KV device to start. Must not already be started. doca_kvdev_set_nguid() must have been called.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kvdev' is NULL, or a required config parameter was not set.
 * - DOCA_ERROR_BAD_STATE - If 'kvdev' is already started.
 * - DOCA_ERROR_NOT_FOUND - If the underlying resource could not be found, or the underlying device NGUID does not match
 *   the value set by doca_kvdev_set_nguid().
 * - DOCA_ERROR_DRIVER - If the underlying resource failed due to transport layer error.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_start(struct doca_kvdev *kvdev);

/**
 * @brief Stop a DOCA KV device.
 *
 * @param [in] kvdev
 * The DOCA KV device to stop. Must be started and have no associated I/O contexts.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kvdev' is NULL.
 * - DOCA_ERROR_BAD_STATE - If 'kvdev' is not started or has associated I/O contexts.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_stop(struct doca_kvdev *kvdev);

/**
 * @brief Check whether the DOCA KV device is started.
 *
 * @param [in] kvdev
 * The DOCA KV device instance to query.
 * @param [out] started
 * Set to 1 if the DOCA KV device is started, 0 otherwise.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kvdev' or 'started' is NULL.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_is_started(const struct doca_kvdev *kvdev, uint8_t *started);

/**
 * @brief Get the size, in bytes, of a DOCA KV device.
 *
 * @param [in] kvdev
 * The DOCA KV device to query. Must be started.
 * @param [out] size
 * The size, in bytes, of the DOCA KV device.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kvdev' or 'size' is NULL.
 * - DOCA_ERROR_BAD_STATE - If 'kvdev' is not started.
 * @note The returned size is valid only while the KV device remains in the started state.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_get_size(const struct doca_kvdev *kvdev, uint64_t *size);

/**
 * @brief Get the maximal key length, in bytes, supported by the DOCA KV device.
 *
 * @param [in] kvdev
 * The DOCA KV device to query. Must be started.
 * @param [out] key_len
 * The maximal key length supported, in bytes.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kvdev' or 'key_len' is NULL.
 * - DOCA_ERROR_BAD_STATE - If 'kvdev' is not started.
 * @note The returned key_len is valid only while the KV device remains in the started state.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_get_max_key_len(const struct doca_kvdev *kvdev, uint16_t *key_len);

/**
 * @brief Get the maximal value length, in bytes, supported by the DOCA KV device.
 *
 * @param [in] kvdev
 * The DOCA KV device to query. Must be started.
 * @param [out] value_len
 * The maximal value length supported, in bytes.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kvdev' or 'value_len' is NULL.
 * - DOCA_ERROR_BAD_STATE - If 'kvdev' is not started.
 * @note The returned value_len is valid only while the KV device remains in the started state.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_get_max_value_len(const struct doca_kvdev *kvdev, uint32_t *value_len);

/**
 * @brief Get the maximum number of tasks supported per I/O context for the DOCA KV device.
 *
 * @param [in] kvdev
 * The DOCA KV device to query. Must be started.
 * @param [out] max_tasks
 * The maximum number of tasks that can be configured per I/O context for this device.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kvdev' or 'max_tasks' is NULL.
 * - DOCA_ERROR_BAD_STATE - If 'kvdev' is not started.
 * @note The returned max_tasks is valid only while the KV device remains in the started state.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_kvdev_get_max_tasks(const struct doca_kvdev *kvdev, uint32_t *max_tasks);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DOCA_KVDEV_H_ */

/** @} */
