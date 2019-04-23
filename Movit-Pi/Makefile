export ROOT := /home/alexmorin/Desktop/movit-cross-compiler

export CROSS_ROOT := $(ROOT)/tools/arm-bcm2708
export COMPILER_ROOT := $(CROSS_ROOT)/arm-linux-gnueabihf
export PATH := $(PATH):$(COMPILER_ROOT)/bin
export SYSROOT_PATH := $(COMPILER_ROOT)/arm-linux-gnueabihf/sysroot
export ROOTFS_PATH := $(ROOT)/rootfs


export TARGET_PREFIX=$(ROOT)/tools/arm-bcm2708/arm-linux-gnueabihf-
export CXX = '/home/alexmorin/Desktop/movit-cross-compiler/tools/arm-bcm2708/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++'

CXXFLAGS = -g -Wall -std=c++11 -O3
LDLIBS = -lpthread -lrt -lm -L/home/alexmorin/Desktop/movit-cross-compiler/rootfs/usr/local/lib -I/home/alexmorin/Desktop/movit-cross-compiler/rootfs/usr/local/include -lbcm2835 -lmosquitto -lmosquittopp
LDFLAGS = -I/home/alexmorin/Desktop/movit-cross-compiler/rootfs/usr/include/arm-linux-gnueabihf -I/home/alexmorin/Desktop/movit-cross-compiler/rootfs/usr/include -I/home/alexmorin/Desktop/movit-cross-compiler/rootfs/usr/local/include -Iexternal/include -Lbcm2835
DIRECTORIES := Obj/movit-pi Executables

.PHONY: default all clean

default: all

OBJECTS-PI = $(patsubst src/movit-pi/%.cpp,Obj/movit-pi/%.o,$(wildcard src/movit-pi/*.cpp))
HEADERS-PI = $(wildcard src/movit-pi/*.h)

Obj/movit-pi/%.o: src/movit-pi/%.cpp $(HEADERS-PI)
	$(CXX) $(CXXFLAGS) -c $< -o Obj/movit-pi/$(notdir $@) $(LDFLAGS)

.PRECIOUS: movit-pi Obj/movit-pi/$(notdir OBJECTS-PI)

movit-pi: $(DIRECTORIES) $(OBJECTS-PI) $(HEADERS-PI)
	$(CXX) $(CXXFLAGS) $(wildcard Obj/movit-pi/*.o) -Wall $(LDLIBS) -o Executables/$@ $(LDFLAGS)

all: movit-pi

$(DIRECTORIES):
	mkdir -p $@

clean:
	-rm -f Obj/movit-pi/*.o
	-rm -f Executables/movit-pi
