CFLAGS=-Wall -ggdb -Isrc -Isrc/lib -Isrc/lib/MosquittoBroker -Isrc/lib/MosquittoBroker/cpp
LDFLAGS=-Lsrc/lib -Lsrc/lib/MosquittoBroker src/lib/MosquittoBroker/cpp/libmosquittopp.so.1 src/lib/MosquittoBroker/libmosquitto.so.1

SOURCES = 	src/main.cpp \
			src/DeviceManager.cpp \
			src/ChairManager.cpp \
			src/BackSeatAngleTracker.cpp \
			src/ForceSensor.cpp \
			src/ForcePlate.cpp \
			src/HumiditySensor.cpp \
			src/Alarm.cpp \
			src/Utils.cpp \
			src/DateTimeRTC.cpp \
			src/lib/MPU6050.cpp \
			src/lib/MAX11611.cpp \
			src/lib/MCP79410.cpp \
			src/lib/PCA9536.cpp \
			src/lib/I2Cdev.cpp \
			src/lib/VL53L0X/VL53L0X.cpp \
			src/lib/BCM2835/bcm2835.c \
			src/MosquittoBroker/MosquittoBroker.cpp \

OBJECTS = $(SOURCES:%.c=%.o)

TARGET = movit-plus

$(TARGET): $(OBJECTS)
	gcc $^ -o $@ -lm -lbcm2835 -lstdc++ -pthread ${LDFLAGS} ${CFLAGS}

%.o: %.c
	gcc -c $< -o $@

clean:
	rm -rf *o $(TARGET)