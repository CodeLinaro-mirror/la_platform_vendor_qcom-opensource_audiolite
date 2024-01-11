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

#endif
