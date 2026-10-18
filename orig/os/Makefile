# Builds the reference binaries: the original 8080 assembly, assembled at
# the 56K CP/M 2.2 addresses, for differential testing against the C port.
# Included from the top-level Makefile, so all paths here are relative to
# the repo root, not to this file's directory.

build/ref/ccp.bin: orig/os/ccp.asm tools/asm80.py
	@mkdir -p build/ref
	$(PYTHON) tools/asm80.py -D origin=0dc00h -o $@ -s build/ref/ccp.sym $<

build/ref/bdos.bin: orig/os/bdos.asm tools/asm80.py
	@mkdir -p build/ref
	$(PYTHON) tools/asm80.py -D origin=0e400h -o $@ -s build/ref/bdos.sym $<
