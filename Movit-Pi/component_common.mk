export TARGET_PREFIX=arm-linux-gnueabihf-

export CC  := $(TARGET_PREFIX)gcc
export CPP := $(TARGET_PREFIX)g++

export ROOT := /usr/local/movit-cross-compiler

export CROSS_ROOT := $(ROOT)/tools/arm-bcm2708
export COMPILER_ROOT := $(CROSS_ROOT)/arm-linux-gnueabihf
export PATH := $(PATH):$(COMPILER_ROOT)/bin
export SYSROOT_PATH := $(COMPILER_ROOT)/arm-linux-gnueabihf/sysroot
export ROOTFS_PATH := $(ROOT)/rootfs
