CFLAGS=-Wall -ggdb -Isrc/lib -Isrc/lib/mosquitto_broker -Isrc/lib/mosquitto_broker/cpp
LDFLAGS=-Lsrc/lib -Lsrc/lib/mosquitto_broker src/lib/mosquitto_broker/cpp/libmosquittopp.so.1 src/lib/mosquitto_broker/libmosquitto.so.1

SOURCES = 	src/main.cpp \
			src/accel_module.cpp \
			src/forceSensor.cpp \
			src/forcePlate.cpp \
			src/init.cpp \
			src/alarm.cpp \
			src/program.cpp \
			src/test.cpp \
			src/lib/MPU6050.cpp \
			src/lib/MAX11611.cpp \
			src/lib/MCP79410.cpp \
			src/lib/PCA9536.cpp \
			src/lib/I2Cdev.cpp \
			src/mosquitto_broker/mosquitto_broker.cpp \

OBJECTS = $(SOURCES:%.c=%.o)

TARGET = movit-plus

$(TARGET): $(OBJECTS)
	gcc $^ -o $@ -lm -lbcm2835 -lstdc++ ${LDFLAGS} ${CFLAGS}

%.o: %.c
	gcc -c $< -o $@

clean:
	rm -rf *o $(TARGET)
