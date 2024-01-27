
# Makefile

# Compiler options
CC = gcc
CFLAGS = -Wall -Wextra -O1

# Source files
# SRCS = main.c mmio.c led.c move.c isr.c motor.c steering_motor.c rover.c
SRCS = main.c pynq_mmio.c pynq_arm_motor_controller.c isr.c arm_motor.c arm.c kinematic_engine.c logger.c claw.c rover.c 

# Object files
OBJS = $(SRCS:.c=.o)

# Executable file
TARGET = roverCore

# Targets
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
