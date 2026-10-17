; HELLO.COM - prints a greeting, the command tail, and the default FCBs
	org	100h
	lxi	sp,stk
	lxi	d,msg
	mvi	c,9
	call	5
	; command tail: length at 80h, text at 81h
	lda	80h
	call	phex
	mvi	e,' '
	mvi	c,2
	call	5
	lxi	h,81h
	lda	80h
	mov	b,a
	ora	a
	jz	notail
tl:	push	h
	push	b
	mov	e,m
	mvi	c,2
	call	5
	pop	b
	pop	h
	inx	h
	dcr	b
	jnz	tl
notail:	mvi	e,'|'
	mvi	c,2
	call	5
	; first 16 bytes of each default FCB
	lxi	h,5ch
	mvi	b,33
fl:	push	h
	push	b
	mov	a,m
	call	phex
	pop	b
	pop	h
	inx	h
	dcr	b
	jnz	fl
	mvi	e,13
	mvi	c,2
	call	5
	mvi	e,10
	mvi	c,2
	call	5
	; user/disk byte
	lda	4
	call	phex
	; warm boot
	jmp	0

phex:	push	psw
	rrc
	rrc
	rrc
	rrc
	call	nib
	pop	psw
nib:	ani	0fh
	adi	90h
	daa
	aci	40h
	daa
	mov	e,a
	mvi	c,2
	call	5
	ret

msg:	db	13,10,"Hello from HELLO.COM",13,10,"$"
	ds	64
stk:
