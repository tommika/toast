# Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License 
# vim: ts=4 sw=4
MODULE_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
include $(MODULE_DIR)/../../etc/Common.mak
$(SRC_DIR)main.c: $(SRC_DIR)gpio.h $(SRC_DIR)interrupts.h $(SRC_DIR)util.h
$(SRC_DIR)gpio.c: $(SRC_DIR)gpio.h
$(SRC_DIR)interrupts.c: $(SRC_DIR)interrupts.h

