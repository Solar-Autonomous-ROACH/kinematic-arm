
# Makefile

# Compiler options
CC = gcc
CFLAGS = -Wall -Wextra -O1

# Source files
# SRCS = main.c mmio.c led.c move.c isr.c motor.c steering_motor.c rover.c
SHARED_SRCS = isr.c  arm_motor.c arm.c kinematic_engine.c logger.c claw.c vision.c
SRCS = main.c pynq_mmio.c pynq_arm_motor_controller.c pynq_rover.c $(SHARED_SRCS)
KRIA_SRCS = main.c kria_mmio.c kria_arm_motor_controller.c kria_rover.c $(SHARED_SRCS)

# Object files
OBJS = $(SRCS:.c=.o)
KRIA_OBJS = $(KRIA_SRCS:.c=.o)

# Executable file
TARGET = roverCore

# Targets
all: $(TARGET)

kria: $(KRIA_OBJS)
	$(CC) $(CFLAGS) $(KRIA_OBJS) -o $(TARGET) -lm

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) -lm

kria_mmio.o: kria_mmio.c
	$(CC) $(CFLAGS) -D KRIA_BOARD='' -c $< -o $@

kria_arm_motor_controller.o: kria_arm_motor_controller.c
	$(CC) $(CFLAGS) -D KRIA_BOARD='' -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(KRIA_OBJS) $(TARGET)
