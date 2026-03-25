LOCAL_PATH := $(call my-dir)
LOCAL_MODULE_DDK_BUILD := true
DLKM_DIR := $(TOP)/device/qcom/common/dlkm


###########################################################
# This is set once per LOCAL_PATH, not per (kernel) module
AUDIOLITE_SRC_FILES := \
	$(wildcard $(LOCAL_PATH)/*) \
	$(wildcard $(LOCAL_PATH)/*/*) \
	$(wildcard $(LOCAL_PATH)/*/*/*)

# Module.symvers needs to be generated as a intermediate module so that
# other modules which depend on ipcc_shmem_test modules can set local
# dependencies to it.

########################### Module.symvers ############################
ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX), gen4_gvm)
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIOLITE_SRC_FILES)
LOCAL_MODULE              := ipcc_shmem_test_module-symvers
LOCAL_MODULE_STEM         := Module.symvers
LOCAL_MODULE_KBUILD_NAME  := Module.symvers
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
endif
ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX), gen4_gvm_sgt)
	include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIOLITE_SRC_FILES)
LOCAL_MODULE              := ipcc_shmem_test_module-symvers
LOCAL_MODULE_STEM         := Module.symvers
LOCAL_MODULE_KBUILD_NAME  := Module.symvers
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
endif

#######################################################################

#KBUILD_OPTIONS
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
