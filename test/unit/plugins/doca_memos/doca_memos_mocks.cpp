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

#include "doca_memos_mocks.h"
#include <cstring>
#include <iostream>

extern "C" {

doca_error_t
doca_nvme_kernel_kvdev_create(const char *device_name,
                              uint8_t *guid,
                              doca_nvme_kernel_kvdev **nvme_kvdev) {
    auto lk = DocaMockControl::lock();
    auto &mock = DocaMockControl::instance();
    if (mock.nvme_kvdev_create_result == DOCA_SUCCESS) {
        *nvme_kvdev = new doca_nvme_kernel_kvdev();
    }
    return mock.nvme_kvdev_create_result;
}

doca_error_t
doca_nvme_kernel_kvdev_add_ns(doca_nvme_kernel_kvdev *nvme_kvdev,
                              char *subnqn,
                              uint16_t ns_id,
                              uint8_t *nguid) {
    return DOCA_SUCCESS;
}

doca_kvdev *
doca_nvme_kernel_kvdev_as_kvdev(doca_nvme_kernel_kvdev *nvme_kvdev) {
    return reinterpret_cast<doca_kvdev *>(nvme_kvdev);
}

doca_error_t
doca_nvme_kernel_kvdev_destroy(doca_nvme_kernel_kvdev *nvme_kvdev) {
    delete nvme_kvdev;
    return DOCA_SUCCESS;
}

doca_error_t
doca_kvdev_start(doca_kvdev *kvdev) {
    auto lk = DocaMockControl::lock();
    return DocaMockControl::instance().kvdev_start_result;
}

doca_error_t
doca_kvdev_stop(doca_kvdev *kvdev) {
    auto lk = DocaMockControl::lock();
    return DocaMockControl::instance().kvdev_stop_result;
}

doca_error_t
doca_kvdev_get_max_tasks(const doca_kvdev *kvdev, uint32_t *max_tasks) {
    if (!kvdev || !max_tasks) return DOCA_ERROR_INVALID_VALUE;
    *max_tasks = 65536;
    return DOCA_SUCCESS;
}

doca_error_t
doca_kvdev_get_max_key_len(const doca_kvdev *kvdev, uint16_t *max_key_len) {
    if (!kvdev || !max_key_len) return DOCA_ERROR_INVALID_VALUE;
    *max_key_len = 16;
    return DOCA_SUCCESS;
}

doca_error_t
doca_pe_create(doca_pe **pe) {
    auto lk = DocaMockControl::lock();
    auto &mock = DocaMockControl::instance();
    if (mock.pe_create_result == DOCA_SUCCESS) {
        *pe = new doca_pe();
    }
    return mock.pe_create_result;
}

doca_error_t
doca_pe_destroy(doca_pe *pe) {
    auto lk = DocaMockControl::lock();
    delete pe;
    return DocaMockControl::instance().pe_destroy_result;
}

uint8_t
doca_pe_progress(doca_pe *pe) {
    auto lk = DocaMockControl::lock();
    auto &mock = DocaMockControl::instance();

    if (!mock.auto_complete_tasks || mock.submitted_tasks.empty()) {
        return mock.pe_progress_return;
    }

    // Drain in submission order (FIFO).  Cap per call to mimic the real PE
    // and to keep a runaway callback (one that re-submits forever) from
    // hanging the test process.
    constexpr size_t max_completions_per_progress = 64;
    size_t drained = 0;
    while (!mock.submitted_tasks.empty() && drained < max_completions_per_progress) {
        auto task = mock.submitted_tasks.front();
        mock.submitted_tasks.erase(mock.submitted_tasks.begin());
        mock.submitted_task_user_data.erase(mock.submitted_task_user_data.begin());

        auto meta_it = mock.task_metadata.find(task);
        if (meta_it == mock.task_metadata.end()) {
            std::cerr << "doca_pe_progress: task " << task << " missing metadata" << std::endl;
            std::abort();
        }
        auto &metadata = meta_it->second;

        doca_data ctx_user_data = {.ptr = nullptr};
        switch (metadata.type) {
        case DocaMockControl::TaskMetadata::STORE:
            if (mock.force_task_error) {
                metadata.status = mock.forced_task_error_code;
                if (mock.task_error_cb) {
                    mock.task_error_cb(task, metadata.user_data, ctx_user_data);
                }
            } else if (mock.task_completion_cb) {
                std::vector<uint8_t> value(metadata.buffer_size);
                memcpy(value.data(), metadata.buffer, metadata.buffer_size);
                mock.kv_store[metadata.key] = value;
                mock.task_completion_cb(task, metadata.user_data, ctx_user_data);
            }
            break;
        case DocaMockControl::TaskMetadata::RETRIEVE:
            if (mock.force_task_error) {
                metadata.status = mock.forced_task_error_code;
                if (mock.task_error_cb) {
                    mock.task_error_cb(task, metadata.user_data, ctx_user_data);
                }
            } else if (mock.task_completion_cb) {
                auto it = mock.kv_store.find(metadata.key);
                if (it != mock.kv_store.end()) {
                    size_t copy_size = std::min(metadata.buffer_size, it->second.size());
                    memcpy(metadata.buffer, it->second.data(), copy_size);
                    mock.task_completion_cb(task, metadata.user_data, ctx_user_data);
                } else if (mock.task_error_cb) {
                    metadata.status = DOCA_ERROR_NOT_FOUND;
                    mock.task_error_cb(task, metadata.user_data, ctx_user_data);
                }
            }
            break;
        case DocaMockControl::TaskMetadata::EXIST:
            // Real DOCA reports EXIST results through the error callback:
            // ALREADY_EXIST when found, NOT_FOUND when absent.
            if (mock.force_task_error) {
                metadata.status = mock.forced_task_error_code;
                if (mock.task_error_cb) {
                    mock.task_error_cb(task, metadata.user_data, ctx_user_data);
                }
            } else if (mock.task_error_cb) {
                auto it = mock.kv_store.find(metadata.key);
                metadata.status =
                    (it != mock.kv_store.end()) ? DOCA_ERROR_ALREADY_EXIST : DOCA_ERROR_NOT_FOUND;
                mock.task_error_cb(task, metadata.user_data, ctx_user_data);
            }
            break;
        }
        drained++;
    }

    return drained > 0 ? 1 : mock.pe_progress_return;
}

doca_error_t
doca_pe_get_notification_handle(const doca_pe *pe, doca_notification_handle_t *handle) {
    auto lk = DocaMockControl::lock();
    auto &mock = DocaMockControl::instance();
    if (mock.pe_get_notification_handle_result == DOCA_SUCCESS) {
        *handle = mock.notification_handle;
    }
    return mock.pe_get_notification_handle_result;
}

doca_error_t
doca_pe_request_notification(doca_pe *pe) {
    auto lk = DocaMockControl::lock();
    return DocaMockControl::instance().pe_request_notification_result;
}

doca_error_t
doca_pe_clear_notification(doca_pe *pe, doca_notification_handle_t handle) {
    auto lk = DocaMockControl::lock();
    return DocaMockControl::instance().pe_clear_notification_result;
}

doca_error_t
doca_pe_connect_ctx(doca_pe *pe, doca_ctx *ctx) {
    auto lk = DocaMockControl::lock();
    return DocaMockControl::instance().pe_connect_ctx_result;
}

doca_error_t
doca_kv_io_create(doca_kvdev *kvdev, doca_kvdev_io **kv_io) {
    auto lk = DocaMockControl::lock();
    auto &mock = DocaMockControl::instance();
    if (mock.kv_io_create_result == DOCA_SUCCESS) {
        *kv_io = new doca_kvdev_io();
    }
    return mock.kv_io_create_result;
}

doca_error_t
doca_kvdev_io_create(doca_kvdev *kvdev, doca_kvdev_io **kv_io) {
    return doca_kv_io_create(kvdev, kv_io);
}

doca_error_t
doca_kv_io_destroy(doca_kvdev_io *kv_io) {
    auto lk = DocaMockControl::lock();
    delete kv_io;
    return DocaMockControl::instance().kv_io_destroy_result;
}

doca_error_t
doca_kvdev_io_destroy(doca_kvdev_io *kv_io) {
    return doca_kv_io_destroy(kv_io);
}

doca_ctx *
doca_kv_io_as_ctx(doca_kvdev_io *kv_io) {
    return reinterpret_cast<doca_ctx *>(kv_io);
}

doca_ctx *
doca_kvdev_io_as_ctx(doca_kvdev_io *kv_io) {
    return doca_kv_io_as_ctx(kv_io);
}

doca_error_t
doca_kvdev_io_set_conf(doca_kvdev_io *kv_io,
                       uint32_t num_tasks,
                       void (*completion_cb)(doca_kv_task *, doca_data, doca_data),
                       void (*error_cb)(doca_kv_task *, doca_data, doca_data)) {
    auto lk = DocaMockControl::lock();
    auto &mock = DocaMockControl::instance();
    mock.task_completion_cb = completion_cb;
    mock.task_error_cb = error_cb;
    return DOCA_SUCCESS;
}

doca_error_t
doca_ctx_start(doca_ctx *ctx) {
    auto lk = DocaMockControl::lock();
    return DocaMockControl::instance().ctx_start_result;
}

doca_error_t
doca_ctx_stop(doca_ctx *ctx) {
    auto lk = DocaMockControl::lock();
    return DocaMockControl::instance().ctx_stop_result;
}

doca_error_t
doca_ctx_set_datapath_on_gpu(doca_ctx *ctx, void *gpu_dev) {
    return DOCA_SUCCESS;
}

doca_error_t
doca_kv_io_set_max_inflight_tasks(doca_kvdev_io *kv_io, uint32_t num_tasks) {
    return DOCA_SUCCESS;
}

doca_error_t
doca_kv_task_alloc_init(doca_kvdev_io *kv_io, doca_kv_task **task) {
    auto lk = DocaMockControl::lock();
    auto &mock = DocaMockControl::instance();

    if (mock.task_alloc_fail_after_n >= 0) {
        if (mock.task_alloc_call_count >= mock.task_alloc_fail_after_n) {
            mock.task_alloc_call_count++;
            *task = nullptr;
            return mock.task_alloc_fail_error;
        }
        mock.task_alloc_call_count++;
    }

    if (mock.kv_task_alloc_result == DOCA_SUCCESS) {
        *task = new doca_kv_task();
    } else {
        *task = nullptr;
    }
    return mock.kv_task_alloc_result;
}

doca_error_t
doca_kv_task_store_set_conf(doca_kv_task *task,
                            doca_data user_data,
                            const void *key,
                            uint32_t key_len,
                            struct iovec *value,
                            uint32_t value_iovcnt) {
    auto lk = DocaMockControl::lock();
    auto &mock = DocaMockControl::instance();

    if (mock.kv_task_store_conf_result == DOCA_SUCCESS) {
        DocaMockControl::TaskMetadata meta;
        meta.type = DocaMockControl::TaskMetadata::STORE;
        meta.key = std::string(static_cast<const char *>(key), key_len);
        meta.buffer = value[0].iov_base;
        meta.buffer_size = value[0].iov_len;
        meta.user_data = user_data;
        mock.task_metadata[task] = meta;
    }
    return mock.kv_task_store_conf_result;
}

doca_error_t
doca_kv_task_retrieve_set_conf(doca_kv_task *task,
                               doca_data user_data,
                               const void *key,
                               uint32_t key_len,
                               struct iovec *value,
                               uint32_t value_iovcnt) {
    auto lk = DocaMockControl::lock();
    auto &mock = DocaMockControl::instance();

    if (mock.kv_task_retrieve_conf_result == DOCA_SUCCESS) {
        DocaMockControl::TaskMetadata meta;
        meta.type = DocaMockControl::TaskMetadata::RETRIEVE;
        meta.key = std::string(static_cast<const char *>(key), key_len);
        meta.buffer = value[0].iov_base;
        meta.buffer_size = value[0].iov_len;
        meta.user_data = user_data;
        mock.task_metadata[task] = meta;
    }
    return mock.kv_task_retrieve_conf_result;
}

doca_error_t
doca_kv_task_exist_set_conf(doca_kv_task *task,
                            doca_data user_data,
                            const void *key,
                            uint32_t key_len) {
    auto lk = DocaMockControl::lock();
    auto &mock = DocaMockControl::instance();

    if (mock.kv_task_exist_conf_result == DOCA_SUCCESS) {
        DocaMockControl::TaskMetadata meta;
        meta.type = DocaMockControl::TaskMetadata::EXIST;
        meta.key = std::string(static_cast<const char *>(key), key_len);
        meta.buffer = nullptr;
        meta.buffer_size = 0;
        meta.user_data = user_data;
        mock.task_metadata[task] = meta;
    }
    return mock.kv_task_exist_conf_result;
}

doca_task *
doca_kv_task_as_task(doca_kv_task *kv_task) {
    return reinterpret_cast<doca_task *>(kv_task);
}

doca_error_t
doca_task_submit(doca_task *task) {
    auto lk = DocaMockControl::lock();
    auto &mock = DocaMockControl::instance();

    if (mock.task_submit_fail_after_n >= 0) {
        if (mock.task_submit_call_count >= mock.task_submit_fail_after_n) {
            mock.task_submit_call_count++;
            return mock.task_submit_fail_error;
        }
        mock.task_submit_call_count++;
    }

    if (mock.task_submit_result == DOCA_SUCCESS) {
        auto kv_task = reinterpret_cast<doca_kv_task *>(task);
        auto meta_it = mock.task_metadata.find(kv_task);
        if (meta_it == mock.task_metadata.end()) {
            std::cerr << "doca_task_submit: task " << kv_task
                      << " submitted without prior set_conf" << std::endl;
            std::abort();
        }
        mock.submitted_tasks.push_back(kv_task);
        mock.submitted_task_user_data.push_back(meta_it->second.user_data);
    }
    return mock.task_submit_result;
}

void
doca_task_free(doca_task *task) {
    auto lk = DocaMockControl::lock();
    auto kv_task = reinterpret_cast<doca_kv_task *>(task);
    auto &mock = DocaMockControl::instance();
    mock.task_metadata.erase(kv_task);
    delete kv_task;
}

doca_error_t
doca_task_get_status(const doca_task *task) {
    auto lk = DocaMockControl::lock();
    auto kv_task = reinterpret_cast<const doca_kv_task *>(task);
    auto &mock = DocaMockControl::instance();
    auto it = mock.task_metadata.find(const_cast<doca_kv_task *>(kv_task));
    if (it != mock.task_metadata.end()) {
        return it->second.status;
    }
    return DOCA_SUCCESS;
}

const char *
doca_error_get_descr(doca_error_t error) {
    switch (error) {
    case DOCA_SUCCESS:
        return "Success";
    case DOCA_ERROR_UNKNOWN:
        return "Unknown error";
    case DOCA_ERROR_INVALID_VALUE:
        return "Invalid value";
    case DOCA_ERROR_NO_MEMORY:
        return "Out of memory";
    case DOCA_ERROR_NOT_FOUND:
        return "Not found";
    case DOCA_ERROR_IO_FAILED:
        return "I/O failed";
    case DOCA_ERROR_BAD_STATE:
        return "Bad state";
    case DOCA_ERROR_OPERATING_SYSTEM:
        return "Operating system error";
    case DOCA_ERROR_ALREADY_EXIST:
        return "Already exists";
    case DOCA_ERROR_FULL:
        return "Full";
    case DOCA_ERROR_IN_PROGRESS:
        return "In progress";
    case DOCA_ERROR_TOO_BIG:
        return "Too big";
    default:
        return "Unrecognized error";
    }
}

} // extern "C"
