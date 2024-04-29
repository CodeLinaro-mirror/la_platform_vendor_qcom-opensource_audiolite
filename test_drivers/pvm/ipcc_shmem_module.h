// SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note
/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#ifndef __IPCC_SHMEM_MOD_H__
#define __IPCC_SHMEM_MOD_H__

#include <linux/ioctl.h>
#include <linux/types.h>

#define IPCC_SHMEM_SIG 44

enum ipc_client {
    IPCC_CLIENT_INVAL = -1,
    IPCC_CLIENT_HOST = 0x1000,
    IPCC_CLIENT_ADSP = 0x2000,
    IPCC_CLIENT_GPDSP0 = 0x3000,
    IPCC_CLIENT_GPDSP1 = 0x4000
};

enum cache_ops {
    CACHE_FLUSH = 1,
    CACHE_INVALIDATE = 2,
};
#define IRQ_ACK                      (0x0001)
#define IRQ_DATA                     (0x0002)
#define IPC_SHMEM_MAGIC              'H'

#define IPCC_CLIENT_HOST_IRQ_ACK     (IPCC_CLIENT_HOST|IRQ_ACK)
#define IPCC_CLIENT_HOST_IRQ_DATA    (IPCC_CLIENT_HOST|IRQ_DATA)
#define IPCC_CLIENT_ADSP_IRQ_ACK     (IPCC_CLIENT_ADSP|IRQ_ACK)
#define IPCC_CLIENT_ADSP_IRQ_DATA    (IPCC_CLIENT_ADSP|IRQ_DATA)
#define IPCC_CLIENT_GPDSP0_IRQ_ACK   (IPCC_CLIENT_GPDSP0|IRQ_ACK)
#define IPCC_CLIENT_GPDSP0_IRQ_DATA  (IPCC_CLIENT_GPDSP0|IRQ_DATA)
#define IPCC_CLIENT_GPDSP1_IRQ_ACK   (IPCC_CLIENT_GPDSP1|IRQ_ACK)
#define IPCC_CLIENT_GPDSP1_IRQ_DATA  (IPCC_CLIENT_GPDSP1|IRQ_DATA)
#define IPCC_IRQ_INVAL               (0xFFFF)

struct ipc_shmem_reg_cb {
    int32_t pid;
};

struct ipc_shmem_cache_ops {
    enum cache_ops flag;
    int32_t size;
    void *buf;
};

struct ipc_shmem_cache_offset_ops {
    enum cache_ops flag;
    uint32_t offset;
    int32_t size;
    void *buf;
};

#define IOCTL_IPC_SHMEM_REG_IRQ \
    _IOWR(IPC_SHMEM_MAGIC, 0x01, struct ipc_shmem_reg_cb)
// Todo: Use single ioctl for cache operations
/*
 * IOCTL_IPC_SHMEM_CACHE_OPS:
 * cache flush or invalidate on the userspace address
 * supported only in the lrh kernel
 */
#define IOCTL_IPC_SHMEM_CACHE_OPS \
    _IOWR(IPC_SHMEM_MAGIC, 0x02, struct ipc_shmem_cache_ops)
/*
 * IOCTL_IPC_SHMEM_CACHE_OFFSET_OPS:
 * cache flush or invalidate on the kernel address
 * User pass the shared memory offset, kernel flush/inval the
 * kernel virtual base address + offset
 */
#define IOCTL_IPC_SHMEM_CACHE_OFFSET_OPS \
    _IOWR(IPC_SHMEM_MAGIC, 0x03, struct ipc_shmem_cache_offset_ops)

#endif
