# Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License 
# vim: ts=4 sw=4
SHELL := /bin/bash
MODULE_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

.DEFAULT_GOAL := all

XARGS:=$(shell command -v gxargs || command -v xargs)

BLD_DIR=$(abspath $(MODULE_DIR)build)/
SRC_DIR=$(MODULE_DIR)src/

S_OBJS=$(patsubst $(SRC_DIR)%.s,$(BLD_DIR)%.o,$(wildcard $(SRC_DIR)*.s))
SS_OBJS=$(patsubst $(SRC_DIR)%.S,$(BLD_DIR)%.o,$(wildcard $(SRC_DIR)*.S))
C_OBJS=$(patsubst $(SRC_DIR)%.c,$(BLD_DIR)%.o,$(wildcard $(SRC_DIR)*.c))
DEPS=$(C_OBJS:%.o=%.d)
OBJS=$(S_OBJS) $(SS_OBJS) $(C_OBJS)
IMAGES=$(BLD_DIR)kernel.img

SAMPLES_DIR=$(MODULE_DIR)samples/
SAMPLES_EXES=$(patsubst $(SAMPLES_DIR)%.c,$(BLD_DIR)samples/%,$(wildcard $(SAMPLES_DIR)*.c))

# Create object files from assembly source files
$(BLD_DIR)%.o: $(SRC_DIR)%.s
	mkdir -p $(BLD_DIR)
	$(TOOLCHAIN)as $< -o $@

# Create object files from C source files
CFLAGS=-std=gnu99 -Wall -Werror -O2 -MMD -D_GNU_SOURCE

$(BLD_DIR)%.o: $(SRC_DIR)%.c
	@mkdir -p $(BLD_DIR)
	$(TOOLCHAIN)gcc $(CFLAGS) -c $< -o $@

all: samples

# Include all .d files
-include $(DEPS)

info:
	@echo "MODULE_DIR:   $(MODULE_DIR)"
	@echo "SRC_DIR:      $(SRC_DIR)"
	@echo "BLD_DIR:      $(BLD_DIR)"
	@echo "OBJS:         $(OBJS)"
	@echo "DEPS:         $(DEPS)"
	@echo "SAMPLES_DIR:  $(SAMPLES_DIR)"
	@echo "SAMPLES_EXES: $(SAMPLES_EXES)"

clean:
	rm -rf $(BLD_DIR)

$(BLD_DIR)bcm.a: $(OBJS)
	$(TOOLCHAIN)ar rcs $@ $(OBJS)

.PHONY: samples
samples: $(SAMPLES_EXES)

CFLAGS_SAMPLES=-std=gnu99 -Wall -Werror -O2
ifeq ($(ARCH),64)
  CFLAGS_SAMPLES+=-m32
endif

$(BLD_DIR)samples/%: $(SAMPLES_DIR)%.c $(BLD_DIR)bcm.a
	@mkdir -p $(BLD_DIR)samples/
	gcc $(CFLAGS_SAMPLES) -o $@ -I $(MODULE_DIR)src $^

