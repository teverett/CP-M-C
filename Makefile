CC      ?= cc
CFLAGS  ?= -std=c99 -Wall -Wextra -O2 -g
PYTHON  ?= python3

SRC = src/host.c src/bdos.c src/ccp.c src/i8080.c

all: build/cpm build/ref/ccp.bin build/ref/bdos.bin

build/cpm: $(SRC) src/cpm.h src/i8080.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(SRC)

# Reference binaries: the original assembly, at the 56K CP/M 2.2 addresses
build/ref/ccp.bin: orig/ccp.asm tools/asm80.py
	@mkdir -p build/ref
	$(PYTHON) tools/asm80.py -D origin=0dc00h -o $@ -s build/ref/ccp.sym $<

build/ref/bdos.bin: orig/bdos.asm tools/asm80.py
	@mkdir -p build/ref
	$(PYTHON) tools/asm80.py -D origin=0e400h -o $@ -s build/ref/bdos.sym $<

test: all
	$(PYTHON) tests/run_tests.py

clean:
	rm -rf build

.PHONY: all test clean
