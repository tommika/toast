# Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License 
# vim: ts=4 sw=4

SHELL := /bin/bash

# Common Makefile
ifndef MODULE_DIR
$(error MODULE_DIR is not set)
endif

.DEFAULT_GOAL := all

XARGS:=$(shell command -v gxargs || command -v xargs)

TOOLCHAIN?=arm-none-eabi-
PART_NAME?=TOAST
PART_PATH?=/media/$(USER)/$(PART_NAME)

ETC_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
VENDOR_DIR:=$(abspath $(ETC_DIR)/../vendor)

BLD_DIR=$(abspath $(MODULE_DIR)build)/
SRC_DIR=$(MODULE_DIR)src/
TEST_SRC_DIR:=$(MODULE_DIR)test/

S_OBJS=$(patsubst $(SRC_DIR)%.s,$(BLD_DIR)%.o,$(wildcard $(SRC_DIR)*.s))
SS_OBJS=$(patsubst $(SRC_DIR)%.S,$(BLD_DIR)%.o,$(wildcard $(SRC_DIR)*.S))
C_OBJS=$(patsubst $(SRC_DIR)%.c,$(BLD_DIR)%.o,$(wildcard $(SRC_DIR)*.c))
DEPS = $(C_OBJS:%.o=%.d)
OBJS=$(S_OBJS) $(SS_OBJS) $(C_OBJS)
IMAGES=$(BLD_DIR)kernel.img

# Create object files from assembly source files
$(BLD_DIR)%.o: $(SRC_DIR)%.s
	mkdir -p $(BLD_DIR)
	$(TOOLCHAIN)as $< -o $@

# Create object files from C source files
$(BLD_DIR)%.o: $(SRC_DIR)%.c
	@mkdir -p $(BLD_DIR)
	$(TOOLCHAIN)gcc -std=c99 -Wall -Werror -O2 -nostdlib -nostartfiles -ffreestanding -MMD -c $< -o $@

all: $(IMAGES)

# Include all .d files
-include $(DEPS)

info:
	@echo "MODULE_DIR: $(MODULE_DIR)"
	@echo "ETC_DIR:    $(ETC_DIR)"
	@echo "VENDOR_DIR: ${VENDOR_DIR}"
	@echo "PART_NAME:  $(PART_NAME)"
	@echo "PART_PATH:  $(PART_PATH)"
	@echo "SRC_DIR:    $(SRC_DIR)"
	@echo "BLD_DIR:    $(BLD_DIR)"
	@echo "OBJS:       $(OBJS)"
	@echo "IMAGES:     $(IMAGES)"
	@echo "DEPS:       $(DEPS)"

install: $(IMAGES)
	@[ -d "$(PART_PATH)" ] || (echo "Please insert SD card, mounted at $(PART_PATH), and then try again" && false)
	cp $(VENDOR_DIR)/bootcode.bin $(PART_PATH)/
	cp $(VENDOR_DIR)/start.elf $(PART_PATH)/
	cp $(IMAGES) $(PART_PATH)/

eject:
	@eject $(PART_PATH) || true

clean:
	rm -rf $(BLD_DIR)

$(BLD_DIR)kernel.img: $(BLD_DIR)kernel.elf
	$(TOOLCHAIN)objcopy $(BLD_DIR)kernel.elf -O binary $@

$(BLD_DIR)kernel.elf: $(OBJS) $(SRC_DIR)kernel.ld
	# Create ELF file
	$(TOOLCHAIN)gcc -nostartfiles $(OBJS) -T $(SRC_DIR)kernel.ld -o $@
	# Dump ELF file, as it is intructive to look at ;-)
	$(TOOLCHAIN)objdump -d $@ > $@.dump

