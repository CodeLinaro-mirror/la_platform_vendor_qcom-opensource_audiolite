LOCAL_PATH := $(call my-dir)
ifeq ($(TARGET_USES_AUDIOLITE), true)
$(warning "Audiolite DLKM Build Enabled", $(TARGET_USES_AUDIOLITE))
DLKM_DIR := $(TOP)/device/qcom/common/dlkm

#KBUILD_OPTIONS
KBUILD_OPTIONS += KERNEL_ROOT=$(shell pwd)/kernel/msm-$(TARGET_KERNEL_VERSION)/
KBUILD_OPTIONS += MODNAME=ipcc_shmem_test_module
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
$(info value of TARGET_USES_KERNEL_PLATFORM IS '$(TARGET_USES_KERNEL_PLATFORM)')

#Clear Environment Variables
include $(CLEAR_VARS)
#Defining the local options
LOCAL_SRC_FILES             :=  \
                                $(LOCAL_PATH)/ipcc_shmem_test_module.c \
                                $(LOCAL_PATH)/Android.mk \
                                $(LOCAL_PATH)/Kbuild
LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)
LOCAL_MODULE              := ipcc_shmem_test_module.ko
LOCAL_MODULE_TAGS         := optional


include $(DLKM_DIR)/Build_external_kernelmodule.mk
endif