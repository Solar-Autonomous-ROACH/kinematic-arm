
# Makefile

# Compiler options
CC = gcc
CFLAGS = -Wall -Wextra -O1

ROVERLIBPATH = ~/SIMBA_C_Driver/rover
ROVERLIBFLAG = -lrover

ARMLIBPATH = ~/kinematic-arm
ARMLIBFLAG = -larm

LDFLAGS = -lm $(ROVERLIBFLAG) $(ARMLIBFLAG)

# Source files
SRCS = main.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable file
TARGET = main


# Targets
all: $(TARGET)

christianenv:
	make ROVERLIBPATH:=~/Documents/projects/capstone/SIMBA_C_Driver/rover ARMLIBPATH:=~/Documents/projects/capstone/kinematic-arm all

$(TARGET): $(OBJS) $(ROVERLIBPATH)/librover.a $(ARMLIBPATH)/lib/libarm.a
	$(CC) $(CFLAGS) -L $(ROVERLIBPATH) -L $(ARMLIBPATH)/lib $(OBJS) -o $(TARGET) $(LDFLAGS)

$(ARMLIBPATH)/lib/libarm.a:
	cd $(ARMLIBPATH) && make 

# Remake the library if needed
.PHONY: $(ARMLIBPATH)/lib/libarm.a

%.o: %.c
	$(CC) $(CFLAGS) -I $(ROVERLIBPATH) -I $(ARMLIBPATH)/include -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
