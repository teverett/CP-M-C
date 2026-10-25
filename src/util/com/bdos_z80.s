	.module bdos_z80
	.globl	_bdos_call

	.area	_CODE
_bdos_call::
	ld	c, a
	call	5
	ld	e, a
	ld	d, #0
	ret

	.globl	_bdos_pointer_call
_bdos_pointer_call::
	ld	c, a
	call	5
	ex	de, hl
	ret
