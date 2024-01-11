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

#define IRQ_ACK                      (0x0001)
#define IRQ_DATA                     (0x0002)

#define IPCC_CLIENT_HOST_IRQ_ACK     (IPCC_CLIENT_HOST|IRQ_ACK)
#define IPCC_CLIENT_HOST_IRQ_DATA    (IPCC_CLIENT_HOST|IRQ_DATA)
#define IPCC_CLIENT_ADSP_IRQ_ACK     (IPCC_CLIENT_ADSP|IRQ_ACK)
#define IPCC_CLIENT_ADSP_IRQ_DATA    (IPCC_CLIENT_ADSP|IRQ_DATA)
#define IPCC_CLIENT_GPDSP0_IRQ_ACK   (IPCC_CLIENT_GPDSP0|IRQ_ACK)
#define IPCC_CLIENT_GPDSP0_IRQ_DATA  (IPCC_CLIENT_GPDSP0|IRQ_DATA)
#define IPCC_CLIENT_GPDSP1_IRQ_ACK   (IPCC_CLIENT_GPDSP1|IRQ_ACK)
#define IPCC_CLIENT_GPDSP1_IRQ_DATA  (IPCC_CLIENT_GPDSP1|IRQ_DATA)
#define IPCC_IRQ_INVAL               (0xFFFF)

typedef struct ipcc_shmem_reg_cb {
    int32_t pid;
}ipcc_shmem_reg_cb_t;

typedef struct ipcc_shmem_dma_buf_info {
    int32_t dma_buf_fd;
}ipcc_shmem_dma_buf_info_t;

#define IPCC_SHMEM_MAGIC     'H'

#define IOCTL_IPCC_SHMEM_REG_IRQ \
    _IOWR(IPCC_SHMEM_MAGIC, 0x01, struct ipcc_shmem_reg_cb)

#define IOCTL_IPCC_SHMEM_GET_FD \
    _IOWR(IPCC_SHMEM_MAGIC, 0x02, struct ipcc_shmem_dma_buf_info)

#define IOCTL_IPCC_SHMEM_ALLOC_DMA_BUF \
    _IOWR(IPCC_SHMEM_MAGIC, 0x03, struct ipcc_shmem_dma_buf_info)

#endif
