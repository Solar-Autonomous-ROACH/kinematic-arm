
# Makefile

# Compiler options
CC = gcc
CFLAGS = -Wall -Wextra -O1

ROVERLIBPATH = ~/SIMBA_C_Driver/rover
ROVERLIBFLAG = -lrover

ARMLIBPATH = ~/kinematic-arm/c
ARMLIBFLAG = -larm

# Source files
SRCS = main.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable file
TARGET = main


# Targets
all: $(TARGET)

christianenv:
	make ROVERLIBPATH:=~/Documents/projects/capstone/SIMBA_C_Driver/rover ARMLIBPATH:=~/Documents/projects/capstone/kinematic-arm/c all


$(TARGET): $(OBJS) $(ROVERLIBPATH)/librover.a $(ARMLIBPATH)/libarm.a
	$(CC) $(CFLAGS) -L $(ROVERLIBPATH) -L $(ARMLIBPATH) $(OBJS) -o $(TARGET) -lm $(ROVERLIBFLAG) $(ARMLIBFLAG)

%.o: %.c
	$(CC) $(CFLAGS) -I $(ROVERLIBPATH) -I $(ARMLIBPATH) -c $< -o $@

clean:
	rm -f $(OBJS) $(KRIA_OBJS) $(TARGET) dummy$(TARGET) libarm.a
