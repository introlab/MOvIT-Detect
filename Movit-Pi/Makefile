SOURCES = 	src/main.cpp \
			src/MPU6050.cpp \
			src/I2Cdev.cpp

OBJECTS = $(SOURCES:%.c=%.o)

TARGET = all

$(TARGET): $(OBJECTS)
	gcc $^ -o $@ -lm -lwiringPi

%.o: %.c
	gcc -c $< -o $@