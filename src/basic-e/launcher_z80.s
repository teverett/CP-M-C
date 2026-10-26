	.module basic_launcher
	.globl _main
	.globl _basic_overlay

	.area _CODE
_launcher_start:
	ld (0x9DF2), sp
	ld hl, #s__DATA
	ld bc, #l__DATA
	ld a, b
	or c
	jr Z, 2$
1$:
	ld (hl), #0
	inc hl
	dec bc
	ld a, b
	or c
	jr NZ, 1$
2$:
	call _main
	ret

_basic_overlay::
	ld hl, #overlay_start
	ld de, #0x9C00
	ld bc, #overlay_end-overlay_start
	ldir
	jp 0x9C00

; Relative branches keep this loader executable after relocation to 9C00h.
overlay_start:
	ld sp, #0x9BF0
	ld hl, #0x0100
3$:
	ld (0x9DF0), hl
	ex de, hl
	ld c, #26
	call 5
	ld de, #0x9D00
	ld c, #20
	call 5
	or a
	jr NZ, 4$
	ld hl, (0x9DF0)
	ld de, #128
	add hl, de
	ld a, h
	cp #0x9B
	jr C, 3$
	jr 5$
4$:
	cp #1
	jr NZ, 5$
	ld hl, (0x9DF0)
	ld de, #0x0100
	or a
	sbc hl, de
	jr Z, 5$
	ld de, #0x9D00
	ld c, #16
	call 5
	cp #255
	jr Z, 5$
	ld de, #0x0080
	ld c, #26
	call 5
	ld sp, (0x9DF2)
	jp 0x0100
5$:
	ld de, #0x9E00
	ld c, #9
	call 5
	ld de, #0x0080
	ld c, #26
	call 5
	ld sp, (0x9DF2)
	ret
overlay_end:

	.area _INITIALIZER
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _DATA
	.area _INITIALIZED
	.area _BSEG
	.area _BSS
	.area _HEAP
