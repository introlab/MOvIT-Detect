include component_common.mk

#System dependencies
LDFLAGS += -lpthread
LDFLAGS += -lrt
LDFLAGS += -lm

MODULE_DIR := $(shell pwd)

CPPFLAGS += -c
CPPFLAGS += -Wall
CXXFLAGS += -std=c++11
CXXFLAGS += -O3

export CPPFLAGS
export CXXFLAGS

export OBJ_DIR = $(MODULE_DIR)/obj
SRC_DIR = $(MODULE_DIR)/src
LIB_DIR = $(MODULE_DIR)/lib

OUTPUT_DIR := $(MODULE_DIR)/output

DIRECTORIES := $(OBJ_DIR) $(OUTPUT_DIR)

# Our lib dependencies
LDFLAGS += $(LIB_DIR)/libmosquittopp.so.1
LDFLAGS += $(LIB_DIR)/libmosquitto.so.1
LDFLAGS += $(LIB_DIR)/libbcm2835.a

# Std lib dependencies
LDFLAGS += $(ROOTFS_PATH)/usr/lib/arm-linux-gnueabihf/libcares.so.2
LDFLAGS += $(ROOTFS_PATH)/usr/lib/arm-linux-gnueabihf/libcrypto.so.1.1
LDFLAGS += $(ROOTFS_PATH)/usr/lib/arm-linux-gnueabihf/libssl.so.1.1
LDFLAGS += $(SYSROOT_PATH)/usr/lib/libdl.so

export TARGET = movit-pi

$(TARGET): | $(DIRECTORIES)
	cd src && $(MAKE) $(TARGET)
	$(CPP) $^ -o $(OUTPUT_DIR)/$(TARGET) $(OBJ_DIR)/*.o $(LDFLAGS)

clean:
	rm -rf $(OUTPUT_DIR)
	rm -rf $(OBJ_DIR)

$(DIRECTORIES):
	mkdir -p $@
