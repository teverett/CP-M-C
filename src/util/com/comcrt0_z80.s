	.module comcrt0_z80
	.globl	_main

	.area	_CODE
_com_start:
	ld	hl, #s__DATA
	ld	bc, #l__DATA
	ld	a, b
	or	a, c
	jr	Z, 1$
	ld	(hl), #0
	dec	bc
	ld	a, b
	or	a, c
	jr	Z, 1$
	ld	d, h
	ld	e, l
	inc	de
	ldir
1$:
	call	_main
	ret

	.area	_INITIALIZER
	.area	_HOME
	.area	_GSINIT
	.area	_GSFINAL
	.area	_DATA
	.area	_INITIALIZED
	.area	_BSEG
	.area	_BSS
	.area	_HEAP
