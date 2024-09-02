/* SPDX-License-Identifier: GPL-2.0 OR BSD-2-Clause */
/*
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __DT_BINDINGS_AUDIOLITE_MAILBOX_IPCC_H
#define __DT_BINDINGS_AUDIOLITE_MAILBOX_IPCC_H

/* Signal IDs for COMPUTE_L0 protocol */
#define IPCC_COMPUTE_L0_SIGNAL_PROCESSING	1
#define IPCC_COMPUTE_L0_SIGNAL_PROCESSING_ACK	2
#define IPCC_COMPUTE_L0_SIGNAL_MSG	3
#define IPCC_COMPUTE_L0_SIGNAL_ACK	4
#define IPCC_COMPUTE_L0_SIGNAL_TUNING	5
#define IPCC_COMPUTE_L0_SIGNAL_TUNING_ACK	6
#define IPCC_COMPUTE_L0_SIGNAL_EVENT	7
#define IPCC_COMPUTE_L0_SIGNAL_EVENT_ACK	8

/* Client IDs */
#define IPCC_CLIENT_APSS_NS1		33
#endif
