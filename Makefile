# Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License 
# vim: ts=4 sw=4
all:
	make -C src/e1-blink clean all
	make -C src/e2-arm-modes clean all
	make -C src/e3-timer clean all
	make -C src/e4-context-switch clean all
	make -C src/e5-monitor-sync clean all
	make -C src/e6-uart clean all
	make -C src/e7-console clean all
