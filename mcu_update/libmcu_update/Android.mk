# Copyright (C) 2012 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


LOCAL_PATH:= $(call my-dir)
# HAL module implemenation, not prelinked and stored in
# hw/<LIGHTS_HARDWARE_MODULE_ID>.<ro.board.platform>.so
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
#LOCAL_PRELINK_MODULE := false
#LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

#LOCAL_FORCE_STATIC_EXECUTABLE := true

#LOCAL_STATIC_LIBRARIES :=    libcutils \
#			    libstdc++ \
#			    libm \
#			    libc


#LOCAL_STATIC_LIBRARIES := 
# LOCAL_C_INCLUDES += bootable/recovery

# LOCAL_STATIC_LIBRARIES := libselinux

LOCAL_SRC_FILES := ib_srec.cpp \
		       UartInterface.cpp \
		       ib_mcuctl.cpp \
		       Protocol.cpp \
		       cobs.cpp \
		       DataFrame.cpp \
			   McuDataFrame.cpp
LOCAL_LDLIBS    := -llog		        
		           
LOCAL_MODULE := libmcu_update
LOCAL_CFLAGS += -Wall
LOCAL_MODULE_TAGS := eng
include $(BUILD_STATIC_LIBRARY)
