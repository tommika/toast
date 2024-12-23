# Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
# vim: ts=4 sw=4
MODULE_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

include $(MODULE_DIR)../../etc/Common.mak

TEST_EXES=$(patsubst $(TEST_SRC_DIR)%.c,$(BLD_DIR)test/%,$(wildcard $(TEST_SRC_DIR)*-test.c))

.PHONY: test
test: $(TEST_EXES)
	@for t in ${TEST_EXES}; do\
		echo "Running test: $$t"; \
		$$t;\
	done

CFLAGS_TEST=-std=c99 -Wall -Werror -O2
#ifeq ($(ARCH),64)
  CFLAGS_TEST+=-m32
#endif

$(BLD_DIR)test/%: $(TEST_SRC_DIR)%.c $(SRC_DIR)proctl.c $(SRC_DIR)assert.c $(SRC_DIR)str.c $(TEST_SRC_DIR)bcm2835-mock.c
	@mkdir -p $(BLD_DIR)/test
	@echo "ARCH: $(ARCH)"
	@echo "CFLAGS_TEST: $(CFLAGS_TEST)"
	gcc $(CFLAGS_TEST) -o $@ -I $(MODULE_DIR)src $^

