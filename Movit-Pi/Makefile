include component_common.mk

#System dependencies
LDFLAGS += -lpthread
LDFLAGS += -lrt
LDFLAGS += -lm

MODULE_DIR := $(shell pwd)

#Common
OBJ_DIR = $(MODULE_DIR)/obj
EXTERNAL_DEP_DIR = $(MODULE_DIR)/external
LIB_DIR = $(EXTERNAL_DEP_DIR)/lib
INC_DIR = $(EXTERNAL_DEP_DIR)/include
OUTPUT_DIR := $(MODULE_DIR)/output

#movit-pi
export OBJ_DIR_MOVIT_PI = $(OBJ_DIR)/movit-pi
SRC_DIR_MOVIT_PI = $(MODULE_DIR)/src/movit-pi

#movit-control
export OBJ_DIR_MOVIT_CONTROL = $(OBJ_DIR)/movit-control
SRC_DIR_MOVIT_CONTROL = $(MODULE_DIR)/src/movit-control

CPPFLAGS += -c
CPPFLAGS += -Wall
CXXFLAGS += -std=c++11
CXXFLAGS += -O3
CXXFLAGS += -I$(INC_DIR)
export CPPFLAGS
export CXXFLAGS

# Our lib dependencies
LDFLAGS += $(LIB_DIR)/libmosquittopp.so.1
LDFLAGS += $(LIB_DIR)/libmosquitto.so.1
LDFLAGS += $(LIB_DIR)/libbcm2835.a

# Std lib dependencies
LDFLAGS += $(ROOTFS_PATH)/usr/lib/arm-linux-gnueabihf/libcares.so.2
LDFLAGS += $(ROOTFS_PATH)/usr/lib/arm-linux-gnueabihf/libcrypto.so.1.1
LDFLAGS += $(ROOTFS_PATH)/usr/lib/arm-linux-gnueabihf/libssl.so.1.1
LDFLAGS += $(SYSROOT_PATH)/usr/lib/libdl.so

export TARGET_MOVIT_PI = movit-pi
export TARGET_MOVIT_CONTROL = movit-control

DIRECTORIES := $(OBJ_DIR_MOVIT_PI) $(OBJ_DIR_MOVIT_CONTROL) $(OUTPUT_DIR)

pi: | $(DIRECTORIES)
	cd $(SRC_DIR_MOVIT_PI) && $(MAKE) $(TARGET_MOVIT_PI)
	$(CPP) $^ -o $(OUTPUT_DIR)/$(TARGET_MOVIT_PI) $(OBJ_DIR_MOVIT_PI)/*.o $(LDFLAGS)

control: | $(DIRECTORIES)
	cd $(SRC_DIR_MOVIT_CONTROL) && $(MAKE) $(TARGET_MOVIT_CONTROL)
	$(CPP) $^ -o $(OUTPUT_DIR)/$(TARGET_MOVIT_CONTROL) $(OBJ_DIR_MOVIT_CONTROL)/*.o $(LDFLAGS)

all: pi control

clean:
	rm -rf $(OUTPUT_DIR)
	rm -rf $(OBJ_DIR)

$(DIRECTORIES):
	mkdir -p $@
