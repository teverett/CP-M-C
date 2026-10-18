CC      ?= cc
CFLAGS  ?= -std=c99 -Wall -Wextra -O2 -g
PYTHON  ?= python3

all: build/cpm build/ref/ccp.bin build/ref/bdos.bin z80 i386

include src/os/Makefile
include orig/os/Makefile

test: all
	$(PYTHON) tests/run_tests.py

clean:
	rm -rf build

.PHONY: all test clean
