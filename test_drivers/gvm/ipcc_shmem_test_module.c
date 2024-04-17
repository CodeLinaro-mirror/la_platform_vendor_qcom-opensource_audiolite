// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/scatterlist.h>
#include <linux/iommu.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/dma-buf.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/version.h>
#include <linux/dma-heap.h>
#include <linux/ktime.h>
#include <linux/build_bug.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/mailbox_client.h>
#include <linux/debugfs.h>
#include <asm/siginfo.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include "ipcc_shmem_module.h"
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,2,0))
#include <linux/sizes.h>
#else
#include <asm-generic/sizes.h>
#endif

#define DMA_HEAP_NAME                      "qcom,ipc_shmem"
#define IPC_SHMEM_DEV_NAME                 "ipc_shmem"
#define DMA_DATA_DIR                        DMA_BIDIRECTIONAL
#define HOST_DEV_NAME                       "ipcc-self-ping-apss_cl0"
#define ADSP_DEV_NAME                       "ipcc-self-ping-adsp_cl0"
#define GPDSP0_DEV_NAME                     "ipcc-self-ping-gpdsp0_cl0"
#define GPDSP1_DEV_NAME                     "ipcc-self-ping-gpdsp1_cl0"
#define SHM_CMD_SYNC                        (0xA5A5)
#define SIZE_1KB                            (1024)
#define SIZE_1MB                            (1024*1024)
/* Shared memory configuration */
#define AUDIOLITE_CARVE_AREA                (0xad000000)
#define AUDIOLITE_CARVE_SIZE                (0x2000000)
#define SHMEM_PHY_ADDR                      (AUDIOLITE_CARVE_AREA)
#define SHMEM_SIZE                          (AUDIOLITE_CARVE_SIZE)
/*
 * shared mem partition
 * LA write area - 10MB
 * LA read area - 10MB
 * Audio config - 1 MB
 */
/* shared memory offset for the LAGVM */
#define SHMEM_WRITE_AREA_SIZE               (0xA00000)    /* 10MB */
#define SHMEM_READ_AREA_SIZE                (0xA00000)    /* 10MB */
#define SHMEM_AUDIO_CONF_AREA_SIZE          (0x100000)    /* 1MB */
#define SHMEM_CMD_AREA_SIZE                 (0x100000)    /* 1MB */
#define SHMEM_DUMMY_AREA_SIZE               (0x100000)    /* 1MB */
#define SHMEM_DMA_LATENCY_AREA_SIZE         (0x100000)    /* 1MB */
#define SHMEM_DSP_PERF_INFO_AREA_SIZE       (0x100000)    /* 1MB */
#define SHMEM_HOST_LA_WRITE_AREA_SIZE       (0x100000)    /* 1MB */
#define SHMEM_HOST_LA_READ_AREA_SIZE        (0x100000)    /* 1MB */
/*
 * Shared memory area to send or receive the cmd, audio-data,
 * audio-hardware configuration with DSPs
 */
#define SHMEM_WRITE_OFFSET                  (0x0)
#define SHMEM_READ_OFFSET                   (SHMEM_WRITE_OFFSET+ \
                                             SHMEM_WRITE_AREA_SIZE)

#define SHMEM_DUMMY_OFFSET                  (SHMEM_READ_OFFSET+ \
                                             SHMEM_READ_AREA_SIZE)

#define SHMEM_CMD_OFFSET                    (SHMEM_DUMMY_OFFSET+ \
                                             SHMEM_DUMMY_AREA_SIZE)

#define SHMEM_AUDIO_CONF_OFFSET             (SHMEM_CMD_OFFSET+ \
                                             SHMEM_CMD_AREA_SIZE)

#define SHMEM_DMA_LATENCY_OFFSET            (SHMEM_AUDIO_CONF_OFFSET+ \
                                             SHMEM_AUDIO_CONF_AREA_SIZE)
/* shared memory area to communicate with host */
#define SHMEM_HOST_LA_WRITE_OFFSET          (SHMEM_DMA_LATENCY_OFFSET + \
                                             SHMEM_DMA_LATENCY_AREA_SIZE)

#define SHMEM_HOST_LA_READ_OFFSET           (SHMEM_HOST_LA_WRITE_OFFSET + \
                                             SHMEM_HOST_LA_WRITE_AREA_SIZE)

#define SHMEM_DSP_PERF_INFO_OFFSET          (SHMEM_HOST_LA_READ_OFFSET+ \
                                             SHMEM_HOST_LA_READ_AREA_SIZE)
/* test data */
#define IPC_SHMEM_TEST_SIZE                 (SHMEM_HOST_LA_WRITE_AREA_SIZE)
#define TEST_DATA                           (0xA5)
#define IPCC_SHMEM_PLATFORM_OFFSET_SHIFT    (40)
/* Set cmd */
#define SHM_CMD_SET_SYNC(shmem_cmd_hdr_p)                     \
        shmem_cmd_hdr_p->sync_word = SHM_CMD_SYNC

#define SHM_CMD_SET_HEADER_SIZE(shmem_cmd_hdr_p, size)        \
        shmem_cmd_hdr_p->header_size = size

#define SHM_CMD_SET_MSG_TYPE(shmem_cmd_hdr_p, type)           \
        shmem_cmd_hdr_p->msg_type = type

#define SHM_CMD_SET_PAYLOAD_SIZE(shmem_cmd_hdr_p, p_size)     \
        shmem_cmd_hdr_p->payload_size = p_size

#define SHM_CMD_SET_OPCODE(shmem_cmd_hdr_p, op_code)          \
        shmem_cmd_hdr_p->opcode = op_code

#define SHM_CMD_SET_DST_CLIENT(shmem_cmd_hdr_p, dst_client)   \
        shmem_cmd_hdr_p->dst_client = dst_client
/* Get cmd */
#define SHM_CMD_GET_SYNC(shmem_cmd_hdr_p)                     \
        shmem_cmd_hdr_p->sync_word

#define SHM_CMD_GET_HEADER_SIZE(shmem_cmd_hdr_p)              \
        shmem_cmd_hdr_p->header_size

#define SHM_CMD_GET_MSG_TYPE(shmem_cmd_hdr_p)                 \
        shmem_cmd_hdr_p->msg_type

#define SHM_CMD_GET_PAYLOAD_SIZE(shmem_cmd_hdr_p)             \
        shmem_cmd_hdr_p->payload_size

#define SHM_CMD_GET_OPCODE(shmem_cmd_hdr_p)                   \
        shmem_cmd_hdr_p->opcode

#define SHM_CMD_GET_DST_CLIENT(shmem_cmd_hdr_p)               \
        shmem_cmd_hdr_p->dst_client

static char *shmem_addr = NULL;
static size_t shmem_alloc_size = 0;
static struct dma_heap *heap = NULL;
static struct dma_buf *dmabuf = NULL;
static struct dma_buf_attachment *buf_attachment = NULL;
static int app_pid = -1;
static int ipc_from_user = 0;

enum shmem_opcode {
    SHMEM_CMD_LOOPBACK,
};

enum shmem_msg_type {
    /* Event message, no ACK requires             */
    SHMEM_MSG_TYPE_EVT = 0,
    /* command type, wait for ACK from the client */
    SHMEM_MSG_TYPE_CMD,
};

struct shmem_memtest {
    uint32_t memtype;
    uint32_t mem_size;
    uint32_t loop_cnt;
};

union payload_ {
    struct shmem_memtest memtest;
};

struct shmem_cmd_hdr {
    int sync_word;
    int header_size;
    uint32_t msg_type;
    int payload_size;
    int opcode;
    /* destination client id in the SHMEM_CMD_DSP_LOOPBACK */
    int dst_client;
    union payload_ payload;
};

struct ipc_shmem_module_data {
    struct ion_client *client;
    struct list_head danglers;
    struct dma_buf_attachment *dmabuf_attach;
};

struct dangling_allocation {
    struct list_head list;
    struct page *page;
    int order;
};

struct ipc_shmem_irq_data {
    const char *name;
    struct dentry *debugfs_dir;
    struct platform_device *pdev;
    struct mbox_client mbox_client;
    struct mbox_chan *mbox_chan;
    u32 pings_sent;
    u32 pings_received;
    u32 data_err;
    u32 cache_mode;
    u32 kernel_test;
    bool is_shemem_alloc;
};
static struct dentry *root_test_dir;
static struct class *ipc_shmem_class;
static int ipc_shmem_major;
static struct device *ipc_shmem_dev;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0))
    static struct iosys_map vmap_struct = {0};
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(5,11,0))
    static struct dma_buf_map vmap_struct = {0};
#else
    static void *vmap_ptr;
#endif  /* LINUX_VERSION_CODE */

static int write_test_data_to_shmem(const char *name,
                                    uint8_t *test_buf,
                                    uint32_t test_buf_size,
                                    uint32_t cache_mode);
static int ipcc_shmem_send_signal_to_user(enum ipc_client client,
                                          bool is_ack);
static void ipc_shmem_disable(void);
static int ipc_shmem_enable(size_t alloc_size, bool is_alloc);
static int ipc_shmem_device_create(void);
static void ipc_shmem_device_destroy(void);

static int shmem_cmd_set_opcode(uint8_t *buffer,
                                uint32_t buffer_size,
                                int opcode,
                                uint32_t payload_size)
{
    struct shmem_cmd_hdr shmem_cmd_hdr_data;
    struct shmem_cmd_hdr *shmem_cmd_hdr_p = &shmem_cmd_hdr_data;

    if(buffer == NULL) {
        return -1;
    }
    if(buffer_size < sizeof(struct shmem_cmd_hdr)) {
        return -1;
    }
    memset(shmem_cmd_hdr_p, 0, sizeof(struct shmem_cmd_hdr));
    SHM_CMD_SET_SYNC(shmem_cmd_hdr_p);
    SHM_CMD_SET_HEADER_SIZE(shmem_cmd_hdr_p, sizeof(struct shmem_cmd_hdr));
    SHM_CMD_SET_MSG_TYPE(shmem_cmd_hdr_p, SHMEM_MSG_TYPE_CMD);
    SHM_CMD_SET_PAYLOAD_SIZE(shmem_cmd_hdr_p, payload_size);
    SHM_CMD_SET_OPCODE(shmem_cmd_hdr_p, opcode);
    memcpy(buffer, shmem_cmd_hdr_p, sizeof(struct shmem_cmd_hdr));

    return 0;
}

static int shmem_cmd_get_header(uint8_t *buffer,
                                struct shmem_cmd_hdr *shmem_cmd_hdr_p)
{
    if(buffer == NULL) {
        return -1;
    }
    if(shmem_cmd_hdr_p == NULL) {
        return -1;
    }
    memset(shmem_cmd_hdr_p, 0, sizeof(struct shmem_cmd_hdr));
    memcpy((uint8_t *)shmem_cmd_hdr_p, buffer, sizeof(struct shmem_cmd_hdr));

    return 0;
}

static int shmem_cmd_log_header_info(uint8_t *buffer)
{

    struct shmem_cmd_hdr shmem_cmd_hdr_data;
    struct shmem_cmd_hdr *shmem_cmd_hdr_p = &shmem_cmd_hdr_data;
    uint32_t msg_type;
    int ret = 0;
    int sync_word;
    int header_size;
    int payload_size;
    int opcode;
    int dst_client = -1;

    ret = shmem_cmd_get_header(buffer, shmem_cmd_hdr_p);
    if(ret != 0) {
        pr_err("%s:  shmem_cmd_get_header failed \n", __func__);
        return -EINVAL;
    }
    sync_word = SHM_CMD_GET_SYNC(shmem_cmd_hdr_p);
    header_size = SHM_CMD_GET_HEADER_SIZE(shmem_cmd_hdr_p);
    msg_type = SHM_CMD_GET_MSG_TYPE(shmem_cmd_hdr_p);
    payload_size = SHM_CMD_GET_PAYLOAD_SIZE(shmem_cmd_hdr_p);
    opcode = SHM_CMD_GET_OPCODE(shmem_cmd_hdr_p);
    dst_client = SHM_CMD_GET_DST_CLIENT(shmem_cmd_hdr_p);

    pr_debug("%s: sync_word=0x%x header_size=%d msg_type=%d\n",
            __FUNCTION__,
            sync_word,
            header_size,
            msg_type);
    pr_debug("%s: payload_size=%d opcode=%d\n",
            __FUNCTION__,
            payload_size,
            opcode);

    return 0;
}

static bool is_ack_message(const char *name)
{
    bool is_ack = false;

    if(strstr(name, "_ack"))
        is_ack = true;
    else
        is_ack = false;

    return is_ack;
}

static enum ipc_client get_ipc_client(const char *name)
{
    enum ipc_client client = IPCC_CLIENT_INVAL;

    if(!strncmp(name, HOST_DEV_NAME, strlen(HOST_DEV_NAME))) {
        client = IPCC_CLIENT_HOST;
    }else if(!strncmp(name, ADSP_DEV_NAME, strlen(ADSP_DEV_NAME))) {
        client = IPCC_CLIENT_ADSP;
    }else if(!strncmp(name, GPDSP0_DEV_NAME, strlen(GPDSP0_DEV_NAME))) {
        client = IPCC_CLIENT_GPDSP0;
    } else if(!strncmp(name, GPDSP1_DEV_NAME, strlen(GPDSP1_DEV_NAME))) {
        client = IPCC_CLIENT_GPDSP1;
    }

    return client;
}

static int begin_cpu_access(uint32_t cache_mode)
{
    if(cache_mode)
        return dma_buf_begin_cpu_access(dmabuf, DMA_DATA_DIR);
    else

    return 0;
}

static int end_cpu_access(uint32_t cache_mode)
{
    if(cache_mode)
        return dma_buf_end_cpu_access(dmabuf, DMA_DATA_DIR);
    else

    return 0;
}

/*
 * IPC_CLIENT_HOST : read the data from shared memory and check the data
 * correctness, send the data TX interrupt back to the HOST
 * DSP_CLIENTS: read the data from shared memory and check the data correctness
 */
static int loopback_data_test(struct ipc_shmem_irq_data *data,
                              uint32_t test_data_size)
{
    uint8_t *shmem_data_addr;
    struct mbox_chan *mbox_chan;
    enum ipc_client client = IPCC_CLIENT_INVAL;
    int ret = 0;
    int i;

    mbox_chan = data->mbox_chan;
    client = get_ipc_client(data->name);
    if(client == IPCC_CLIENT_INVAL) {
        pr_err("%s:  Invalid client \n", __func__);
        return -EINVAL;
    }
    if(test_data_size > SHMEM_READ_AREA_SIZE) {
        pr_err("%s:  Invalid test size \n", __func__);
        return -EINVAL;
    }

    if(client == IPCC_CLIENT_HOST) {
        shmem_data_addr = shmem_addr + SHMEM_HOST_LA_READ_OFFSET;
    }else {
        shmem_data_addr = shmem_addr + SHMEM_READ_OFFSET;
    }
    pr_info("%s: cache_mode %d\n", __FUNCTION__, data->cache_mode);

    ret = begin_cpu_access(data->cache_mode);
    if (ret) {
        pr_err("%s: begin_cpu_access()() failed with %d\n",
               __FUNCTION__, ret);
    }
    for (i = 0; i < test_data_size; i++) {
        if (shmem_data_addr[i] != TEST_DATA) {
            ret = -EFAULT;
        }
    }
    ret = end_cpu_access(data->cache_mode);
    if (ret) {
        pr_err("%s: end_cpu_access() failed with %d\n",
               __FUNCTION__, ret);
    }

    if(ret == -EFAULT) {
        pr_err("%s:  Data comparison fail \n", __func__);
        data->data_err++;
    }else {
        pr_info("%s:  Data comparison success test_data_size=%d \n",
                __func__,
                test_data_size);
    }
    if(client == IPCC_CLIENT_HOST) {
        if(data->kernel_test == 0x31) {
            write_test_data_to_shmem(data->name,
                                    shmem_data_addr,
                                    test_data_size,
                                    data->cache_mode);

            ret = mbox_send_message(mbox_chan, "foo_data");
            if (ret < 0) {
                pr_err("Failed to send mbox data: %d \n", ret);
                return ret;
            }
            mbox_client_txdone(mbox_chan, 0);
        }
    }

    return ret;
}

static irqreturn_t ipc_shmem_irq_fn(int irq, void *d)
{
    struct ipc_shmem_irq_data *data = d;
    struct platform_device *pdev = data->pdev;
    enum ipc_client client = IPCC_CLIENT_INVAL;
    bool is_ack = false;

    data->pings_received++;
    dev_dbg(&pdev->dev, "IRQ received: %d : %s \n", irq, data->name);

    if(!is_ack_message(data->name)) {
        is_ack = false;
        if(data->kernel_test)
            loopback_data_test(data, IPC_SHMEM_TEST_SIZE);
    }else {
        is_ack = true;
        dev_dbg(&pdev->dev, "ACK received: %d : %s \n", irq, data->name);
    }
    if(0 <= app_pid) {
        client = get_ipc_client(data->name);
        if(client == IPCC_CLIENT_INVAL) {
            pr_err("%s:  Invalid client \n", __func__);
            return IRQ_HANDLED;
        }
        ipcc_shmem_send_signal_to_user(client, is_ack);
    }

    return IRQ_HANDLED;
}

static int write_test_data_to_shmem(const char *name,
                                    uint8_t *test_buf,
                                    uint32_t test_buf_size,
                                    uint32_t cache_mode)
{
    int ret = 0;
    enum ipc_client client = IPCC_CLIENT_INVAL;
    uint8_t *shmem_cmd_addr;
    uint8_t *shmem_data_addr;

    client = get_ipc_client(name);
    if(client == IPCC_CLIENT_INVAL) {
        pr_err("%s:  Invalid client \n", __func__);
        return -EINVAL;
    }
    if(test_buf_size > SHMEM_HOST_LA_WRITE_AREA_SIZE) {
        pr_err("%s:  Invalid test size \n", __func__);
        return -EINVAL;
    }
    if(client == IPCC_CLIENT_HOST) {
        shmem_cmd_addr = NULL;
        shmem_data_addr = shmem_addr + SHMEM_HOST_LA_WRITE_OFFSET;
    }else {
        shmem_cmd_addr = shmem_addr + SHMEM_CMD_OFFSET;
        shmem_data_addr = shmem_addr + SHMEM_WRITE_OFFSET;
    }
    pr_info("%s: cache_mode %d shmem_cmd_addr=0x%x shmem_data_addr=0x%x\n",
            __FUNCTION__,
            cache_mode,
            shmem_cmd_addr,
            shmem_data_addr);
    ret = begin_cpu_access(cache_mode);
    if (ret) {
        pr_err("%s: begin_cpu_access()() failed with %d\n", __FUNCTION__, ret);
    }
    /* write data to shared memory */
    if(shmem_cmd_addr != NULL)    {
        shmem_cmd_set_opcode(shmem_cmd_addr,
                             SHMEM_SIZE,
                             SHMEM_CMD_LOOPBACK,
                             IPC_SHMEM_TEST_SIZE);
        shmem_cmd_log_header_info(shmem_cmd_addr);
    }
    if(shmem_data_addr != NULL) {
        if(test_buf == NULL)
            memset(shmem_data_addr, TEST_DATA, test_buf_size);
        else
            memcpy(shmem_data_addr, test_buf, test_buf_size);
    }
    ret = end_cpu_access(cache_mode);
    if (ret) {
        pr_err("%s: end_cpu_access() failed with %d\n", __FUNCTION__, ret);
    }

    return ret;
}

static ssize_t
ipc_shmem_irq_debugfs_write(struct file *filp,
                            const char __user *buf,
                            size_t count, loff_t *ppos)
{
    int ret = 0;
    struct ipc_shmem_irq_data *data = filp->private_data;
    struct platform_device *pdev;
    struct mbox_chan *mbox_chan;

    if (!data) {
        pr_err("%s: Failed to get the driver_data\n", __func__);
        return -EFAULT;
    }
    pdev = data->pdev;
    mbox_chan = data->mbox_chan;
    pr_info("%s: %s \n", __FUNCTION__, data->name);
    if(data->kernel_test) {
        if(!data->is_shemem_alloc) {
            ret = ipc_shmem_enable(SHMEM_SIZE, true);
            if (ret) {
                pr_err("shared memory enable fail : %d\n", ret);
                return ret;
            }
            data->is_shemem_alloc = true;
        }
        write_test_data_to_shmem(data->name,
                                 NULL,
                                 IPC_SHMEM_TEST_SIZE,
                                 data->cache_mode);
    }
    ret = mbox_send_message(mbox_chan, "foo_data");
    if (ret < 0) {
        dev_err(&pdev->dev, "Failed to send mbox data: %d\n", ret);
        return ret;
    }
    data->pings_sent++;
    mbox_client_txdone(mbox_chan, 0);

    dev_info(&pdev->dev, "mbox message sent successfully test size=%d \n",
             IPC_SHMEM_TEST_SIZE);

    return count;
}

static struct file_operations ipc_shmem_irq_debugfs_fops = {
    .open = simple_open,
    .write = ipc_shmem_irq_debugfs_write
};

static int ipc_shmem_irq_probe(struct platform_device *pdev)
{
    int irq, ret;
    struct dentry *dentry;
    struct ipc_shmem_irq_data *data;

    pr_debug("%s:  \n", __FUNCTION__);
    data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;
    data->pdev = pdev;
    data->name = pdev->dev.of_node->name;
    irq = platform_get_irq(pdev, 0);
    if (irq < 0) {
        dev_err(&pdev->dev, "Failed to get irq. ret: %d\n", irq);
        return irq;
    }
    ret = devm_request_threaded_irq(&pdev->dev,
                    irq, ipc_shmem_irq_fn,
                     NULL, IRQF_ONESHOT,
                    data->name, data);
    if (ret) {
        dev_err(&pdev->dev,
            "Failed to request interrupt: ret: %d\n", ret);
        return ret;
    }
    data->mbox_client.dev = &pdev->dev;
    data->mbox_client.knows_txdone = true;
    data->mbox_chan = mbox_request_channel(&data->mbox_client, 0);
    if (IS_ERR(data->mbox_chan)) {
        dev_err(&pdev->dev, "Failed to allocate mbox channel\n");
        return PTR_ERR(data->mbox_chan);
    }
    data->debugfs_dir = debugfs_create_dir(data->name, root_test_dir);
    if (!data->debugfs_dir) {
        dev_err(&pdev->dev, "Failed to create debugfs directory\n");
        ret = -ENOMEM;
        goto debugfs_dir_err;
    }
    dentry = debugfs_create_file("ping", 0200, data->debugfs_dir, data,
                    &ipc_shmem_irq_debugfs_fops);
    if (!dentry) {
        dev_err(&pdev->dev, "Failed to create the ping file\n");
        ret = -ENOMEM;
        goto debugfs_file_err;
    }
    debugfs_create_u32("pings_sent", 0600, data->debugfs_dir,
                    &data->pings_sent);
    debugfs_create_u32("pings_received", 0600, data->debugfs_dir,
                    &data->pings_received);
    debugfs_create_u32("data_err", 0600, data->debugfs_dir,
                    &data->data_err);
    debugfs_create_u32("cache_mode", 0600, data->debugfs_dir,
                    &data->cache_mode);
    debugfs_create_u32("kernel_test", 0600, data->debugfs_dir,
                    &data->kernel_test);
    platform_set_drvdata(pdev, data);
    dev_info(&pdev->dev, "Probed %s with IRQ: %d\n", data->name, irq);

    return 0;

debugfs_file_err:
    debugfs_remove_recursive(data->debugfs_dir);
debugfs_dir_err:
    mbox_free_channel(data->mbox_chan);
    return ret;
}

static int ipc_shmem_irq_remove(struct platform_device *pdev)
{
    struct ipc_shmem_irq_data *data = platform_get_drvdata(pdev);

    debugfs_remove_recursive(data->debugfs_dir);
    mbox_free_channel(data->mbox_chan);
    dev_info(&pdev->dev, "Removed %s\n", data->name);

    return 0;
}

static const struct of_device_id ipc_shmem_irq_of_match[] = {
    { .compatible = "qcom,ipcc-self-ping"},
    {}
};
MODULE_DEVICE_TABLE(of, ipc_shmem_irq_of_match);

static struct platform_driver ipc_shmem_irq_driver = {
    .probe = ipc_shmem_irq_probe,
    .remove = ipc_shmem_irq_remove,
    .driver = {
        .name = "qcom_ipcc_self_ping",
        .of_match_table = ipc_shmem_irq_of_match,
    },
};

static int ipc_shmem_irq_init(void)
{
    int ret = 0;

    pr_debug("%s: \n", __FUNCTION__);
    root_test_dir = debugfs_create_dir("ipc_shmem_irq", NULL);
    if (!root_test_dir)
        return -ENOMEM;
    ret = platform_driver_register(&ipc_shmem_irq_driver);
    if (ret) {
        debugfs_remove_recursive(root_test_dir);
        return ret;
    }
    pr_debug("%s: done \n", __FUNCTION__);

    return ret;
}

static void ipc_shmem_irq_exit(void)
{
    platform_driver_unregister(&ipc_shmem_irq_driver);
    debugfs_remove_recursive(root_test_dir);
}

static void free_all_danglers(struct list_head *danglers)
{
    struct list_head *iter, *tmp;
    list_for_each_safe(iter, tmp, danglers) {
        struct dangling_allocation *dangler =
            container_of(iter, struct dangling_allocation, list);
        __free_pages(dangler->page, dangler->order);
        list_del(iter);
        kfree(dangler);
    }
}

static void ipc_shmem_disable(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,11,0))
    if(shmem_addr != NULL) {
        dma_buf_vunmap(dmabuf, &vmap_struct);
        shmem_addr = NULL;
    }
#else
    if(shmem_addr != NULL) {
        dma_buf_vunmap(dmabuf, vmap_ptr);
        shmem_addr = NULL;
    }
#endif
    shmem_alloc_size = 0;
    if(buf_attachment != NULL) {
        dma_buf_detach(dmabuf, buf_attachment);
        buf_attachment = NULL;
    }
    if(dmabuf != NULL) {
        dma_buf_put(dmabuf);
        dmabuf = NULL;
    }
}

static int ipc_shmem_enable(size_t alloc_size, bool is_alloc)
{
    int ret = 0;

    pr_debug("%s: alloc_size=%d\n", __FUNCTION__, alloc_size);

    if (!ipc_shmem_dev) {
        pr_err("no device found\n");
        return -ENODEV;
    }
    if(is_alloc) {
        /* buffer alloc will be done in the driver */
        heap = dma_heap_find(DMA_HEAP_NAME);
        if (!heap) {
            pr_err("%s: %s heap not found\n", __FUNCTION__, DMA_HEAP_NAME);
            return PTR_ERR(heap);
        }

        dmabuf = dma_heap_buffer_alloc(heap, alloc_size, O_RDWR | O_CLOEXEC, 0);
        if (IS_ERR(dmabuf)) {
            pr_err("%s: Failed to allocate dma-buf, alloc_size=%d, err=%d\n",
                    __FUNCTION__, alloc_size,  PTR_ERR(dmabuf));
            return PTR_ERR(dmabuf);
        }
        shmem_alloc_size = alloc_size;
    }else {
        if (dmabuf == NULL) {
            pr_err("%s: dmabuf not allocated \n", __FUNCTION__);
            return -ENODEV;
        }
    }
    buf_attachment = dma_buf_attach(dmabuf, ipc_shmem_dev);
    if (IS_ERR(buf_attachment)) {
        pr_err("%s: dma_buf_attach() failed with %d\n",
                __FUNCTION__, PTR_ERR(buf_attachment));
        ret = PTR_ERR(buf_attachment);
        goto free_dmabuf;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,11,0))
    ret = dma_buf_vmap(dmabuf, &vmap_struct);
    if (ret) {
        pr_err("%s: failed to vmap() a dmabuf, failed with %d\n",
                __FUNCTION__, ret);
        goto unmap;
    }
    if (!vmap_struct.is_iomem)
        shmem_addr = vmap_struct.vaddr;
    else
        shmem_addr = vmap_struct.vaddr_iomem;
#else
    shmem_addr = dma_buf_vmap(dmabuf);
    if (!shmem_addr) {
        pr_err("%s: failed to vmap() a dmabuf\n", __FUNCTION__);
        goto unmap;
    }
#endif
    if(shmem_addr != NULL)    {
        pr_info("%s: dma_buf_vmap success \n", __FUNCTION__);
    }else {
        pr_err("%s: shmem_addr is NULL \n", __FUNCTION__);
    }

    return ret;

unmap:
    if(buf_attachment != NULL) {
        dma_buf_detach(dmabuf, buf_attachment);
        buf_attachment = NULL;
    }

free_dmabuf:
    if(dmabuf != NULL) {
        dma_buf_put(dmabuf);
        dmabuf = NULL;
        shmem_alloc_size = 0;
    }

    return ret;
}

static int ipcc_shmem_send_signal_to_user(enum ipc_client client,
                                          bool is_ack)
{
    struct kernel_siginfo info = {0};
    struct task_struct *t = NULL;
    int ret = 0;

    info.si_signo = IPCC_SHMEM_SIG;
    info.si_code = SI_QUEUE;
    if(is_ack)
        info.si_int = client | IRQ_ACK;
    else
        info.si_int = client | IRQ_DATA;

    rcu_read_lock();
    t = pid_task(find_pid_ns(app_pid, &init_pid_ns), PIDTYPE_PID);
    rcu_read_unlock();
    if(t == NULL) {
        pr_err("ipcc_shmem_send_signal_to_user: Invalid callback info \n");
        ret = -EINVAL;
    }else {
        ret = send_sig_info(IPCC_SHMEM_SIG, &info, t);
        if(ret < 0)
            pr_err("ipcc_shmem_send_signal_to_user: send signal fails %d \n",
                    ret);
    }

    return ret;
}

static long ipc_shmem_ioctl(struct file *file,
                            unsigned cmd,
                            unsigned long arg)
{
    int ret = 0;
    ipcc_shmem_reg_cb_t cb_info;

    pr_debug("ipc_shmem_ioctl: cmd:0x%x \n", cmd);
    switch (cmd)
    {
        case IOCTL_IPCC_SHMEM_REG_IRQ:
            /* Register the application pid */
            pr_debug("ipc_shmem_ioctl: IOCTL_IPCC_SHMEM_REG_IRQ \n");
            if(copy_from_user(&cb_info,
                            (void __user *)arg,
                            sizeof(ipcc_shmem_reg_cb_t))) {
                ret = -EFAULT;
            }else {
                app_pid = cb_info.pid;
            }
        break;
        case IOCTL_IPCC_SHMEM_GET_FD:
        case IOCTL_IPCC_SHMEM_ALLOC_DMA_BUF:
        default:
            pr_err("command not supported \n");
            return -EINVAL;
    }

    return ret;
}

static int ipc_shmem_open(struct inode *inode, struct file *file)
{
    struct ipc_shmem_module_data *module_data = kzalloc(
        sizeof(struct ipc_shmem_module_data), GFP_KERNEL);

    if (!module_data)
        return -ENOMEM;
    INIT_LIST_HEAD(&module_data->danglers);
    file->private_data = module_data;
    pr_debug("ipc_shmem device opened\n");

    return 0;
}

static int ipc_shmem_release(struct inode *inode, struct file *file)
{
    struct ipc_shmem_module_data *module_data = file->private_data;

    free_all_danglers(&module_data->danglers);
    kfree(module_data);
    pr_debug("ipc_shmem device closed\n");

    return 0;
}

static ssize_t ipc_shmem_read(struct file *filep, char *buffer,
    size_t length, loff_t *offset)
{
    if(shmem_addr == NULL)
        return 0;
    if(length > SHMEM_SIZE)
        length = SHMEM_SIZE;
    if(copy_to_user((void __user *)buffer, shmem_addr, length))
        return -EFAULT;
    pr_debug("%s: done length=%d\n", __FUNCTION__, length);

    return length;
}

static int ipc_shmem_mmap(struct file *filp, struct vm_area_struct *vma)
{
    u64 req_len, pgoff, req_start;
    unsigned long phy_addr = SHMEM_PHY_ADDR;
    unsigned long size = SHMEM_SIZE;
    int ret = 0;

    req_len = vma->vm_end - vma->vm_start;
    pgoff = vma->vm_pgoff &
        ((1U << (IPCC_SHMEM_PLATFORM_OFFSET_SHIFT - PAGE_SHIFT)) - 1);
    req_start = pgoff << PAGE_SHIFT;

    if (size < PAGE_SIZE || req_start + req_len > size)
        return -EINVAL;
    /* map the area as writecombine/uncached */
    vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
    vma->vm_pgoff = (phy_addr >> PAGE_SHIFT) + pgoff;

    pr_debug("%s: phy_addr=0x%lx pgoff=0x%lx, size=%d\n",
            __FUNCTION__, phy_addr, vma->vm_pgoff, req_len);
    ret = remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
                   req_len, vma->vm_page_prot);
    if (ret != 0) {
        pr_err("%s: memory map failed ret=%d\n", __FUNCTION__, ret);
        return ret;
    }

    return ret;
}

static ssize_t ipc_shmem_write(struct file *filep, const char *buffer,
    size_t length, loff_t *offset)
{
    if(shmem_addr == NULL)
        return 0;
    if(length > SHMEM_SIZE)
        length = SHMEM_SIZE;
    if(copy_from_user(shmem_addr, (void __user *)buffer, length))
        return -EFAULT;
    pr_debug("%s: done length=%d\n", __FUNCTION__, length);

    return length;
}

static const struct file_operations ipc_shmem_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = ipc_shmem_ioctl,
    .open = ipc_shmem_open,
    .release = ipc_shmem_release,
    .read = ipc_shmem_read,
    .write = ipc_shmem_write,
    .mmap = ipc_shmem_mmap,
};

static int ipc_shmem_device_create(void)
{
    int rc = 0;

    ipc_shmem_major = register_chrdev(0, IPC_SHMEM_DEV_NAME,
                                      &ipc_shmem_fops);
    if (ipc_shmem_major < 0) {
        rc = ipc_shmem_major;
        pr_err("Unable to register chrdev: %d\n", ipc_shmem_major);
        goto out;
    }
    ipc_shmem_class = class_create(THIS_MODULE, IPC_SHMEM_DEV_NAME);
    if (IS_ERR(ipc_shmem_class)) {
        rc = PTR_ERR(ipc_shmem_class);
        pr_err("Unable to create class: %d\n", rc);
        goto err_create_class;
    }
    ipc_shmem_dev = device_create(ipc_shmem_class, NULL,
                    MKDEV(ipc_shmem_major, 0),
                    NULL, IPC_SHMEM_DEV_NAME);
    if (IS_ERR(ipc_shmem_dev)) {
        rc = PTR_ERR(ipc_shmem_dev);
        pr_err("Unable to create device: %d\n", rc);
        goto err_create_device;
    }
    /*
     * Set both the streaming DMA mask and the coherent DMA mask
     * Inform the kernel about devices DMA addressing capabilities
     */
    rc = dma_coerce_mask_and_coherent(ipc_shmem_dev, DMA_BIT_MASK(64));
    if (rc) {
        pr_err("Unable to set dma_masks for device: %d\n", rc);
        goto err_destroy_device;
    }
    return rc;

err_destroy_device:
    device_destroy(ipc_shmem_class, MKDEV(ipc_shmem_major, 0));
err_create_device:
    class_destroy(ipc_shmem_class);
err_create_class:
    unregister_chrdev(ipc_shmem_major, IPC_SHMEM_DEV_NAME);
out:
    return rc;
}

static void ipc_shmem_device_destroy(void)
{
    ipc_shmem_disable();
    device_destroy(ipc_shmem_class, MKDEV(ipc_shmem_major, 0));
    class_destroy(ipc_shmem_class);
    unregister_chrdev(ipc_shmem_major, IPC_SHMEM_DEV_NAME);
}

static int ipc_shmem_init(void)
{
    int rc = 0;

    pr_debug("%s: ipc_from_user =%d \n",
            __FUNCTION__, ipc_from_user);
    rc = ipc_shmem_device_create();
    if(rc) {
        pr_err("%s: ipc_shmem_device_create　failed rc=%d \n",
            __FUNCTION__, rc);
        return rc;
    }
    if(!ipc_from_user) {
        rc = ipc_shmem_irq_init();
        if(rc) {
            pr_err("%s: ipc_shmem_irq_init　failed rc=%d \n",
                __FUNCTION__, rc);
            return rc;
        }
    }
    return rc;
}

static void ipc_shmem_exit(void)
{
    ipc_shmem_device_destroy();
    if(!ipc_from_user) {
        ipc_shmem_irq_exit();
    }
    return;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,16,0))
MODULE_IMPORT_NS(DMA_BUF);
#endif
MODULE_PARM_DESC(ipc_from_user, "Access IPCC from user space");
module_param(ipc_from_user,int,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("QCOM Test Driver for Shared Memory & IPCC");
module_init(ipc_shmem_init);
module_exit(ipc_shmem_exit);
