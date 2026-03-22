/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DOCA_MEMOS_MOCKS_H
#define DOCA_MEMOS_MOCKS_H

#include <stdint.h>
#include <sys/uio.h> // For struct iovec

// Mock DOCA types - C-compatible
typedef int doca_error_t;
typedef int doca_notification_handle_t;

#define DOCA_SUCCESS 0
#define DOCA_ERROR_UNKNOWN 1
#define DOCA_ERROR_INVALID_VALUE 6
#define DOCA_ERROR_NO_MEMORY 7
#define DOCA_ERROR_NOT_FOUND 16
#define DOCA_ERROR_IO_FAILED 17
#define DOCA_ERROR_BAD_STATE 18
#define DOCA_ERROR_OPERATING_SYSTEM 20
#define DOCA_ERROR_ALREADY_EXIST 23
#define DOCA_ERROR_FULL 24
#define DOCA_ERROR_IN_PROGRESS 26
#define DOCA_ERROR_TOO_BIG 27

#define DOCA_KVDEV_NAME_LEN 256
#define DOCA_KVDEV_GUID_LEN 16
#define DOCA_NVME_KERNEL_KVDEV_SUBNQN_LEN 256

#define doca_event_invalid_handle (-1)

// Mock structures - C-compatible (empty structs as placeholders)
struct doca_kvdev {};

struct doca_kvdev_io {};

struct doca_pe {};

struct doca_ctx {};

struct doca_task {};

struct doca_kv_task {};

struct doca_nvme_kernel_kvdev {};

union doca_data {
    void *ptr;
    uint64_t u64;
};

#ifdef __cplusplus
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

// Mock control structure - C++ only
class DocaMockControl {
public:
    static DocaMockControl &
    instance() {
        static DocaMockControl inst;
        return inst;
    }

    // Serialises every singleton access. Recursive so callbacks fired from
    // doca_pe_progress() can re-enter mocks (e.g. doca_task_free) without
    // self-deadlock. Tests should hold this lock around any field they read
    // or mutate when a threaded progress engine is alive.
    static std::unique_lock<std::recursive_mutex>
    lock() {
        return std::unique_lock<std::recursive_mutex>(instance().mutex_);
    }

    // Configuration for mock behavior - all DOCA API return values
    doca_error_t nvme_kvdev_create_result = DOCA_SUCCESS;
    doca_error_t kvdev_start_result = DOCA_SUCCESS;
    doca_error_t kvdev_stop_result = DOCA_SUCCESS;
    doca_error_t pe_create_result = DOCA_SUCCESS;
    doca_error_t pe_destroy_result = DOCA_SUCCESS;
    doca_error_t kv_io_create_result = DOCA_SUCCESS;
    doca_error_t kv_io_destroy_result = DOCA_SUCCESS;
    doca_error_t ctx_start_result = DOCA_SUCCESS;
    doca_error_t ctx_stop_result = DOCA_SUCCESS;
    doca_error_t pe_connect_ctx_result = DOCA_SUCCESS;
    doca_error_t kv_task_alloc_result = DOCA_SUCCESS;
    doca_error_t kv_task_store_conf_result = DOCA_SUCCESS;
    doca_error_t kv_task_retrieve_conf_result = DOCA_SUCCESS;
    doca_error_t kv_task_exist_conf_result = DOCA_SUCCESS;
    doca_error_t task_submit_result = DOCA_SUCCESS;
    doca_error_t pe_get_notification_handle_result = DOCA_SUCCESS;
    doca_error_t pe_request_notification_result = DOCA_SUCCESS;
    doca_error_t pe_clear_notification_result = DOCA_SUCCESS;

    // Advanced error injection - fail after N successful calls
    int task_alloc_fail_after_n =
        -1; // -1 = never fail, 0 = fail first call, N = fail after N calls
    int task_alloc_call_count = 0;
    doca_error_t task_alloc_fail_error = DOCA_ERROR_FULL;
    int task_submit_fail_after_n = -1;
    int task_submit_call_count = 0;
    doca_error_t task_submit_fail_error = DOCA_ERROR_FULL;

    // Force specific task to fail with specific error
    bool force_task_error = false;
    doca_error_t forced_task_error_code = DOCA_ERROR_NOT_FOUND;

    // Callbacks. The plugin only uses doca_kvdev_io_set_conf, which installs
    // a single completion/error pair for all task types, so the mock keeps a
    // single pair too.
    using TaskCompletionCb = std::function<void(doca_kv_task *, doca_data, doca_data)>;
    using TaskErrorCb = std::function<void(doca_kv_task *, doca_data, doca_data)>;

    TaskCompletionCb task_completion_cb = nullptr;
    TaskErrorCb task_error_cb = nullptr;

    // Simulated KV storage
    std::map<std::string, std::vector<uint8_t>> kv_store;

    // Submitted tasks tracking
    std::vector<doca_kv_task *> submitted_tasks;
    std::vector<doca_data> submitted_task_user_data;

    // Progress tracking
    int pe_progress_return = 0; // 0 = no progress, 1 = progress made
    bool auto_complete_tasks = false; // Automatically complete tasks on progress

    // Notification handle
    doca_notification_handle_t notification_handle = 100; // Mock FD

    // Reset all state. Acquires the mock lock so callers don't have to;
    // since the lock is recursive this is safe to call from a context that
    // already holds it.
    void
    reset() {
        std::unique_lock<std::recursive_mutex> guard(mutex_);
        nvme_kvdev_create_result = DOCA_SUCCESS;
        kvdev_start_result = DOCA_SUCCESS;
        kvdev_stop_result = DOCA_SUCCESS;
        pe_create_result = DOCA_SUCCESS;
        pe_destroy_result = DOCA_SUCCESS;
        kv_io_create_result = DOCA_SUCCESS;
        kv_io_destroy_result = DOCA_SUCCESS;
        ctx_start_result = DOCA_SUCCESS;
        ctx_stop_result = DOCA_SUCCESS;
        pe_connect_ctx_result = DOCA_SUCCESS;
        kv_task_alloc_result = DOCA_SUCCESS;
        kv_task_store_conf_result = DOCA_SUCCESS;
        kv_task_retrieve_conf_result = DOCA_SUCCESS;
        kv_task_exist_conf_result = DOCA_SUCCESS;
        task_submit_result = DOCA_SUCCESS;
        pe_get_notification_handle_result = DOCA_SUCCESS;
        pe_request_notification_result = DOCA_SUCCESS;
        pe_clear_notification_result = DOCA_SUCCESS;

        task_alloc_fail_after_n = -1;
        task_alloc_call_count = 0;
        task_alloc_fail_error = DOCA_ERROR_FULL;
        task_submit_fail_after_n = -1;
        task_submit_call_count = 0;
        task_submit_fail_error = DOCA_ERROR_FULL;
        force_task_error = false;
        forced_task_error_code = DOCA_ERROR_NOT_FOUND;

        task_completion_cb = nullptr;
        task_error_cb = nullptr;

        kv_store.clear();
        submitted_tasks.clear();
        submitted_task_user_data.clear();
        task_metadata.clear();

        pe_progress_return = 0;
        auto_complete_tasks = false;
        notification_handle = 100;
    }

    // Task metadata storage
    struct TaskMetadata {
        enum Type { STORE, RETRIEVE, EXIST } type;

        std::string key;
        void *buffer;
        size_t buffer_size;
        doca_data user_data;
        doca_error_t status = DOCA_SUCCESS;
    };

    std::map<doca_kv_task *, TaskMetadata> task_metadata;

private:
    DocaMockControl() = default;

    std::recursive_mutex mutex_;
};
#endif // __cplusplus

// Mock DOCA function declarations - C-compatible
#ifdef __cplusplus
extern "C" {
#endif

doca_error_t
doca_nvme_kernel_kvdev_create(const char *device_name,
                              uint8_t *guid,
                              doca_nvme_kernel_kvdev **nvme_kvdev);
doca_error_t
doca_nvme_kernel_kvdev_add_ns(doca_nvme_kernel_kvdev *nvme_kvdev,
                              char *subnqn,
                              uint16_t ns_id,
                              uint8_t *nguid);
doca_kvdev *
doca_nvme_kernel_kvdev_as_kvdev(doca_nvme_kernel_kvdev *nvme_kvdev);
doca_error_t
doca_nvme_kernel_kvdev_destroy(doca_nvme_kernel_kvdev *nvme_kvdev);
doca_error_t
doca_kvdev_start(doca_kvdev *kvdev);
doca_error_t
doca_kvdev_stop(doca_kvdev *kvdev);
doca_error_t
doca_kvdev_get_max_tasks(const doca_kvdev *kvdev, uint32_t *max_tasks);
doca_error_t
doca_kvdev_get_max_key_len(const doca_kvdev *kvdev, uint16_t *max_key_len);

doca_error_t
doca_pe_create(doca_pe **pe);
doca_error_t
doca_pe_destroy(doca_pe *pe);
uint8_t
doca_pe_progress(doca_pe *pe);
doca_error_t
doca_pe_get_notification_handle(const doca_pe *pe, doca_notification_handle_t *handle);
doca_error_t
doca_pe_request_notification(doca_pe *pe);
doca_error_t
doca_pe_clear_notification(doca_pe *pe, doca_notification_handle_t handle);
doca_error_t
doca_pe_connect_ctx(doca_pe *pe, doca_ctx *ctx);

doca_error_t
doca_kv_io_create(doca_kvdev *kvdev, doca_kvdev_io **kv_io);
doca_error_t
doca_kvdev_io_create(doca_kvdev *kvdev, doca_kvdev_io **kv_io); // Alias
doca_error_t
doca_kv_io_destroy(doca_kvdev_io *kv_io);
doca_error_t
doca_kvdev_io_destroy(doca_kvdev_io *kv_io); // Alias
doca_ctx *
doca_kv_io_as_ctx(doca_kvdev_io *kv_io);
doca_ctx *
doca_kvdev_io_as_ctx(doca_kvdev_io *kv_io); // Alias
doca_error_t
doca_kvdev_io_set_conf(doca_kvdev_io *kv_io,
                       uint32_t num_tasks,
                       void (*completion_cb)(doca_kv_task *, doca_data, doca_data),
                       void (*error_cb)(doca_kv_task *, doca_data, doca_data));

doca_error_t
doca_ctx_start(doca_ctx *ctx);
doca_error_t
doca_ctx_stop(doca_ctx *ctx);
doca_error_t
doca_ctx_set_datapath_on_gpu(doca_ctx *ctx, void *gpu_dev);

doca_error_t
doca_kv_io_set_max_inflight_tasks(doca_kvdev_io *kv_io, uint32_t num_tasks);

doca_error_t
doca_kv_task_alloc_init(doca_kvdev_io *kv_io, doca_kv_task **task);
doca_error_t
doca_kv_task_store_set_conf(doca_kv_task *task,
                            doca_data user_data,
                            const void *key,
                            uint32_t key_len,
                            struct iovec *value,
                            uint32_t value_iovcnt);
doca_error_t
doca_kv_task_retrieve_set_conf(doca_kv_task *task,
                               doca_data user_data,
                               const void *key,
                               uint32_t key_len,
                               struct iovec *value,
                               uint32_t value_iovcnt);
doca_error_t
doca_kv_task_exist_set_conf(doca_kv_task *task,
                            doca_data user_data,
                            const void *key,
                            uint32_t key_len);

doca_task *
doca_kv_task_as_task(doca_kv_task *kv_task);
doca_error_t
doca_task_submit(doca_task *task);
void
doca_task_free(doca_task *task);
doca_error_t
doca_task_get_status(const doca_task *task);

const char *
doca_error_get_descr(doca_error_t error);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // DOCA_MEMOS_MOCKS_H
