SOURCES = 	src/main.cpp \
			src/MPU6050.cpp \
			src/I2Cdev.cpp

OBJECTS = $(SOURCES:%.c=%.o)

TARGET = movit-plus

$(TARGET): $(OBJECTS)
	gcc $^ -o $@ -lm -lbcm2835

%.o: %.c
	gcc -c $< -o $@

clean:
	rm -rf *o $(TARGET)