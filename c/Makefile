
# Makefile

# Compiler options
CC = gcc
CFLAGS = -Wall -Wextra -O1

LIBPATH = ~/SIMBA_C_Driver/rover
ROVERLIBFLAG = -lrover

# Source files
# SRCS = main.c mmio.c led.c move.c isr.c motor.c steering_motor.c rover.c
SHARED_SRCS = arm_motor_controller.c rover_isr.c arm_motor.c arm.c kinematic_engine.c logger.c claw.c vision.c
SRCS = $(SHARED_SRCS)

# Object files
OBJS = $(SRCS:.c=.o)

# Executable file
TARGET = roverCore

# Targets
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -L $(LIBPATH) $(OBJS) -o $(TARGET) -lm $(ROVERLIBFLAG)

%.o: %.c
	$(CC) $(CFLAGS) -I $(LIBPATH) -c $< -o $@

clean:
	rm -f $(OBJS) $(KRIA_OBJS) $(TARGET)
