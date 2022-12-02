LOCAL_PATH := $(call my-dir)

# Build static library.
# ----------------------------------------------
include $(CLEAR_VARS)

# Some global macros.
LOCAL_MACRO := -D_LINUX_ANDROID_ -D__ANDROID__ -D_LINUX_ -D_SUPPORT_64BITS_ -D_CUST_PLAT_

ifeq ($(TARGET_GPU_API), vulkan)
LOCAL_MACRO += -DPATHFINDER_USE_VULKAN
endif

# Additional include path.
INCLUDE_PATH := $(LOCAL_PATH)/../../third_party

# Source files.
SRC_LIST := $(wildcard $(LOCAL_PATH)/../../src/common/*.cpp)
SRC_LIST += $(wildcard $(LOCAL_PATH)/../../src/common/math/*.cpp)
SRC_LIST += $(wildcard $(LOCAL_PATH)/../../src/gpu/*.cpp)

ifeq ($(TARGET_GPU_API), vulkan)
SRC_LIST += $(wildcard $(LOCAL_PATH)/../../src/gpu/vk/*.cpp)
else
SRC_LIST += $(wildcard $(LOCAL_PATH)/../../src/gpu/gl/*.cpp)
endif

SRC_LIST += $(wildcard $(LOCAL_PATH)/../../src/core/*.cpp)
SRC_LIST += $(wildcard $(LOCAL_PATH)/../../src/core/paint/*.cpp)
SRC_LIST += $(wildcard $(LOCAL_PATH)/../../src/core/data/*.cpp)
SRC_LIST += $(wildcard $(LOCAL_PATH)/../../src/core/d3d9/*.cpp)
SRC_LIST += $(wildcard $(LOCAL_PATH)/../../src/core/d3d9/data/*.cpp)
SRC_LIST += $(wildcard $(LOCAL_PATH)/../../src/core/d3d11/*.cpp)

# We don't need Platform and SwapChain on Android.
SRC_LIST := $(filter-out $(LOCAL_PATH)/../../src/gpu/gl/platform.cpp, $(SRC_LIST))
SRC_LIST := $(filter-out $(LOCAL_PATH)/../../src/gpu/gl/swap_chain.cpp, $(SRC_LIST))
SRC_LIST := $(filter-out $(LOCAL_PATH)/../../src/gpu/vk/platform.cpp, $(SRC_LIST))
SRC_LIST := $(filter-out $(LOCAL_PATH)/../../src/gpu/vk/swap_chain.cpp, $(SRC_LIST))

LOCAL_MODULE            := pathfinder_static
LOCAL_MODULE_FILENAME   := libpathfinder # Prefix "lib" is necessary.
LOCAL_SRC_FILES         := $(SRC_LIST:$(LOCAL_PATH)/%=%)
LOCAL_ARM_MODE          := arm
LOCAL_C_INCLUDES        := $(INCLUDE_PATH)

LOCAL_CFLAGS            := $(LOCAL_MACRO) -fpic -fPIC -O2 -mfpu=neon

# Additional C flags for armeabi-v7a.
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_CFLAGS            += -march=armv7-a -mthumb
endif

# C++ flags.
LOCAL_CPPFLAGS          := $(LOCAL_CFLAGS) -O3 -pipe -w -fno-exceptions -fno-rtti
LOCAL_CPPFLAGS          += -std=c++14 -fexceptions

# Set this variable to true when your module has a very high number of
# sources and/or dependent static or shared libraries.
LOCAL_SHORT_COMMANDS    := true

include $(BUILD_STATIC_LIBRARY)
# ----------------------------------------------
