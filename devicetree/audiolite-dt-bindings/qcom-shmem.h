/* SPDX-License-Identifier: GPL-2.0 OR BSD-2-Clause */
/*
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __DT_BINDINGS_SHARED_MEMORY_H
#define __DT_BINDINGS_SHARED_MEMORY_H

/* VMIDs */
#define AC_VM_SSC_Q6			0x5
#define VMID_ADSP_Q6			0x6
#define VMID_GPDSP1_Q6			0x2C

/* Access permissions */
#define QCOM_PERM_READ			0x4
#define QCOM_PERM_WRITE			0x2
#define QCOM_PERM_EXEC			0x1
#define QCOM_PERM_RW			0x6
#define QCOM_PERM_RWX			0x7

#endif
