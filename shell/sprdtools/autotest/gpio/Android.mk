LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(SHARKLJ1_BBAT_BIT64), true)
LOCAL_32_BIT_ONLY := false
else
LOCAL_32_BIT_ONLY := true
endif

LOCAL_SRC_FILES := gpio.cpp

LOCAL_MODULE := libgpio
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := autotest

#BBAT GPIO ADDRESS
ifeq ($(SHARKL2_BBAT_GPIO), true)
LOCAL_CFLAGS += -DSHARKL2_BBAT_GPIO_ADDRESS
endif

ifeq ($(SHARKLJ1_BBAT_GPIO), true)
LOCAL_CFLAGS += -DSHARKL2_BBAT_GPIO_ADDRESS
endif

ifeq ($(iSHARKL2_BBAT_GPIO), true)
LOCAL_CFLAGS += -DISHARKL2_BBAT_GPIO_ADDRESS
endif

ifeq ($(IWHALE2_BBAT_GPIO), true)
LOCAL_CFLAGS += -DIWHALE2_BBAT_GPIO_ADDRESS
endif


LOCAL_C_INCLUDES:= \
     $(TOP)/vendor/sprd/proprietories-source/autotest

LOCAL_SHARED_LIBRARIES:= \
    libcutils \
	liblog

include $(BUILD_SHARED_LIBRARY)