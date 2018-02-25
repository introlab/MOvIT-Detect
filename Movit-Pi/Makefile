CFLAGS=-Wall -ggdb -Isrc/lib/mosquitto_broker -Isrc/lib/mosquitto_broker/cpp
LDFLAGS=-Lsrc/lib/mosquitto_broker src/lib/mosquitto_broker/cpp/libmosquittopp.so.1 src/lib/mosquitto_broker/libmosquitto.so.1

SOURCES = 	src/main.cpp \
			src/mosquitto_broker/mosquitto_broker.cpp \
			src/lib/MPU6050.cpp \
			src/lib/I2Cdev.cpp \

OBJECTS = $(SOURCES:%.c=%.o)

TARGET = movit-plus

$(TARGET): $(OBJECTS)
	gcc $^ -o $@ -lm -lbcm2835 -lstdc++ ${LDFLAGS}

%.o: %.c
	gcc -c $< -o $@ ${CFLAGS}

clean:
	rm -rf *o $(TARGET)