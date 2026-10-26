CC      ?= cc
CFLAGS  ?= -std=c99 -Wall -Wextra -O2 -g
PYTHON  ?= python3

all: build/cpm build/ref/ccp.bin build/ref/bdos.bin z80 i386 z80-util-coms utility-disk

include src/os/Makefile
include orig/os/Makefile

test: all z80-util-coms utility-disk
	$(PYTHON) tests/run_tests.py
	$(PYTHON) tests/test_ed.py
	$(PYTHON) tests/test_util.py
	$(PYTHON) tests/test_basic.py
	$(PYTHON) tests/test_basic_int.py

test-basic: all
	$(PYTHON) tests/test_basic.py
	$(PYTHON) tests/test_basic_int.py

test-ed: all z80-util-coms utility-disk
	$(PYTHON) tests/test_ed.py

test-util: all z80-util-coms utility-disk
	$(PYTHON) tests/test_util.py

clean:
	rm -rf build

.PHONY: all test test-ed test-util test-basic clean
