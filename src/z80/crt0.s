; crt0.s - startup code and entry stubs for the native Z80 build (SDCC)
;
; Memory map (see cpm.h):  0000  reset / CP/M low memory
;                          0100  TPA
;                          A000  CCP data     A800  BDOS entry page
;                          A900  compiled code (this image)

	.module crt0

	.globl	_cpm_start
	.globl	_wboot_c
	.globl	_ccp_entry_c
	.globl	_ccpclear_entry_c
	.globl	_bdos_entry_c
	.globl	_ent_c
	.globl	_ent_de
	.globl	_ent_hl
	.globl	_ent_sp
	.globl	_bdos_stack

STACK_TOP	= 0xFFF0
BDOS_STACK	= 512

	.area	_HEADER (ABS)
	.org	0x0000
reset:
	di
	ld	sp, #STACK_TOP
	jp	init

	.area	_CODE
init:
	; clear the (uninitialised) data area
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
	call	_cpm_start
2$:	halt
	jr	2$

; BDOS entry: C = function, DE = information address.  Returns A=L, B=H,
; HL = result.  Runs on the BDOS stack; IX and IY are preserved.
_bdos_entry::
	ld	(_ent_sp), sp
	ld	sp, #_bdos_stack + BDOS_STACK
	push	ix
	push	iy
	ld	(_ent_de), de
	ld	a, c
	ld	(_ent_c), a
	call	_bdos_entry_c
	pop	iy
	pop	ix
	ld	sp, (_ent_sp)
	ld	hl, (_ent_hl)
	ld	a, l
	ld	b, h
	ret

; Warm boot (JMP 0 lands here via the BIOS jump table)
_wboot_entry::
	ld	sp, #STACK_TOP
	call	_wboot_c
	jp	_wboot_entry

; CCP entry points, reached by a program jumping to CCP_BASE / CCP_BASE+3
; with C = user/disk byte
_ccp_entry::
	ld	sp, #STACK_TOP
	ld	a, c
	ld	(_ent_c), a
	call	_ccp_entry_c
_ccpclear_entry::
	ld	sp, #STACK_TOP
	ld	a, c
	ld	(_ent_c), a
	call	_ccpclear_entry_c

; Unimplemented BIOS entries called directly by a program
_bios_stub::
	xor	a, a
	ld	h, a
	ld	l, a
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
