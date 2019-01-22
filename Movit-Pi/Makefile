export TARGET_PREFIX=arm-linux-gnueabihf-
export CXX = $(TARGET_PREFIX)g++

CXXFLAGS = -g -Wall -std=c++11 -O3
LDLIBS = -lpthread -lrt -lm -lmosquitto -lbcm2835 -lmosquittopp
LDFLAGS = -I/usr/include -I/usr/local/include -Iexternal/include -Lexternal/lib -lmosquitto -lbcm2835 -lmosquittopp
DIRECTORIES := Obj/movit-pi Obj/movit-control Executables

.PHONY: default all clean

default: all

OBJECTS-PI = $(patsubst src/movit-pi/%.cpp,Obj/movit-pi/%.o,$(wildcard src/movit-pi/*.cpp))
HEADERS-PI = $(wildcard src/movit-pi/*.h)

OBJECTS-CONTROL = $(patsubst src/movit-control/%.cpp,Obj/movit-control/%.o,$(wildcard src/movit-control/*.cpp))
HEADERS-CONTROL = $(wildcard src/movit-control/*.h)

Obj/movit-pi/%.o: src/movit-pi/%.cpp $(HEADERS-PI)
	$(CXX) $(CXXFLAGS) -c $< -o Obj/movit-pi/$(notdir $@) $(LDFLAGS)

Obj/movit-control/%.o: src/movit-control/%.cpp $(HEADERS-CONTROL)
	$(CXX) $(CXXFLAGS) -c $< -o Obj/movit-control/$(notdir $@) $(LDFLAGS)

.PRECIOUS: movit-pi movit-control Obj/movit-pi/$(notdir OBJECTS-PI) Obj/movit-control/$(notdir OBJECTS-CONTROL)

movit-pi: $(DIRECTORIES) $(OBJECTS-PI) $(HEADERS-PI)
	$(CXX) $(CXXFLAGS) $(wildcard Obj/movit-pi/*.o) -Wall $(LDLIBS) -o Executables/$@ $(LDFLAGS)

movit-control: $(DIRECTORIES) $(OBJECTS-CONTROL) $(HEADERS-CONTROL)
	$(CXX) $(CXXFLAGS) $(wildcard Obj/movit-control/*.o) -Wall $(LDLIBS) -o Executables/$@ $(LDFLAGS)

all: movit-pi movit-control

$(DIRECTORIES):
	mkdir -p $@

clean:
	-rm -f Obj/movit-pi/*.o Obj/movit-control/*.o
	-rm -f Executables/movit-pi Executables/movit-control
