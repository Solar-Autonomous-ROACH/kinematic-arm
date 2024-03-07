
# Makefile

# Compiler options
CC = gcc
CFLAGS = -Wall -Wextra -O1
# Check if the operating system is macOS
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
BIN_DIR := bin
LIB_DIR := lib

ifeq ($(shell uname), Darwin)
    ROVERLIBPATH ?= ~/Documents/projects/capstone/SIMBA_C_Driver/rover
else
    ROVERLIBPATH ?= /home/ubuntu/SIMBA_C_Driver/rover
endif
LDFLAGS := -lm -LROVERLIBPATH $(ROVERLIBFLAG)
INCLUDEFLAGS := -I $(ROVERLIBPATH) -I $(INC_DIR)

ifeq ($(VISION_DUMMY), 1)
	CFLAGS += -DVISION_DUMMY
endif
ifeq ($(BAD_KRIA), 1)
	CFLAGS += -DBAD_KRIA
endif
ifeq ($(OLD_ROVER), 1)
	CFLAGS += -DOLD_ROVER
endif

.PHONY: all clean allclean

## Library Building
LIB_NAME := libarm.a
LIB_SRCS := $(wildcard $(SRC_DIR)/*.c)
LIB_INCFILES := $(wildcard $(INC_DIR)/*.h)
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SRCS))

all: $(LIB_DIR)/$(LIB_NAME)

$(LIB_DIR)/$(LIB_NAME): $(LIB_OBJS) | $(LIB_DIR)
	ar r $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) -c $< -o $@

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
## End Library Building

## Cleans
cleanbuild:
	rm -rf build

cleanbin:
	rm -rf bin

cleanlib:
	rm -rf lib

clean: cleanbuild cleanbin cleanlib
## End Cleans
