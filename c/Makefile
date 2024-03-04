
# Makefile

# Compiler options
CC = gcc
CFLAGS = -Wall -Wextra -O1

LIBPATH = ~/SIMBA_C_Driver/rover
ROVERLIBFLAG = -lrover

# Source files
LIBSRCS = arm_motor_api.c arm_motor.c arm.c kinematic_engine.c logger.c claw.c vision.c
SRCS = $(LIBSRCS) main.c

# Object files
OBJS = $(SRCS:.c=.o)
LIBOBJS = $(LIBSRCS:.c=.o)

# Executable file
TARGET = arm

LIBNAME = libarm.a

# Targets
all: $(TARGET) $(LIBNAME)

dummy:
	make CFLAGS+=-DVISION_DUMMY TARGET=dummy$(TARGET) all

christianenv:
	make LIBPATH:=~/Documents/projects/capstone/SIMBA_C_Driver/rover all


$(LIBNAME): $(LIBOBJS)
	ar r $(LIBNAME) $(LIBOBJS)

$(TARGET): $(OBJS) $(LIBPATH)/librover.a
	$(CC) $(CFLAGS) -L $(LIBPATH) $(OBJS) -o $(TARGET) -lm $(ROVERLIBFLAG)

%.o: %.c
	$(CC) $(CFLAGS) -I $(LIBPATH) -c $< -o $@

clean:
	rm -f $(OBJS)

allclean: clean
	rm -f $(TARGET) dummy$(TARGET) libarm.a
