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
 * @file doca_nvme_kernel_kvdev_io.h
 * @page doca_nvme_kernel_kvdev_io
 * @defgroup DOCA_NVME_KERNEL_KVDEV_IO DOCA NVMe Kernel KV I/O Context
 * DOCA NVMe kernel backend I/O context for key-value operations.
 *
 * @{
 */
#ifndef DOCA_NVME_KERNEL_KVDEV_IO_H_
#define DOCA_NVME_KERNEL_KVDEV_IO_H_

#include <doca_compat.h>
#include <doca_error.h>

#include <doca_kvdev_io.h>
#include <doca_nvme_kernel_kvdev.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque structure representing a DOCA NVMe kernel KV I/O context.
 * This structure is thread-unsafe and is typically bound to a core.
 * This structure extends struct doca_kvdev_io.
 * This structure is used by KV applications and services.
 */
struct doca_nvme_kernel_kvdev_io;

/*********************************************************************************************************************
 * DOCA NVMe Kernel KV IO Context API
 *********************************************************************************************************************/

/**
 * @brief Create a DOCA NVMe kernel KV I/O context.
 *
 * @details The I/O context transport queue capacity is derived from the number of tasks configured via
 * doca_kvdev_io_set_num_tasks(). doca_kvdev_io_set_num_tasks() must be called before starting the I/O context.
 *
 * @param [in] kkvdev
 * The associated DOCA NVMe kernel KV device. Must be started.
 * @param [out] kio
 * The created DOCA NVMe kernel KV I/O context.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kkvdev' or 'kio' is NULL.
 * - DOCA_ERROR_BAD_STATE - If 'kkvdev' is not started.
 * - DOCA_ERROR_NO_MEMORY - failed to allocate memory.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_nvme_kernel_kvdev_io_create(struct doca_nvme_kernel_kvdev *kkvdev,
					      struct doca_nvme_kernel_kvdev_io **kio);

/**
 * @brief Free a DOCA NVMe kernel KV I/O context.
 *
 * @param [in] kio
 * The I/O context to release. Must be idle.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - If 'kio' is NULL.
 * - DOCA_ERROR_BAD_STATE - I/O context is not idle. Use doca_ctx_stop() to stop it.
 */
DOCA_EXPERIMENTAL
doca_error_t doca_nvme_kernel_kvdev_io_destroy(struct doca_nvme_kernel_kvdev_io *kio);

/**
 * @brief Convert DOCA NVMe kernel KV I/O context instance into DOCA KV I/O context.
 *
 * @param [in] kio
 * DOCA NVMe kernel KV I/O context instance. This must remain valid until after the DOCA KV I/O context is no longer
 * required.
 *
 * @return
 * DOCA KV I/O context upon success, NULL otherwise.
 */
DOCA_EXPERIMENTAL
struct doca_kvdev_io *doca_nvme_kernel_kvdev_io_as_kvdev_io(struct doca_nvme_kernel_kvdev_io *kio);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DOCA_NVME_KERNEL_KVDEV_IO_H_ */

/** @} */
