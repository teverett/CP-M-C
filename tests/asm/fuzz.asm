; FUZZ.COM - table driven BDOS exerciser.  A table of calls is appended to
; the program by the test generator.  Record format:
;   func, dumpflag, addr(lo,hi), nbytes, repeat, bytes...  (func 0ffh ends)
; nbytes bytes are copied to addr, then the call is made `repeat` times
; with DE = addr.
; The DMA buffer (80h..0ffh) is filled with a running pattern first.
; After the call it prints: func A B HL checksum(80h..0ffh) [36 bytes at addr]
	org	100h
	lxi	sp,stk
	lxi	h,table
	shld	tp
	xra	a
	sta	cnt
next:	lhld	tp
	mov	a,m
	cpi	0ffh
	jz	done
	sta	func
	inx	h
	mov	a,m
	sta	dump
	inx	h
	mov	e,m
	inx	h
	mov	d,m
	inx	h
	xchg
	shld	faddr
	xchg
	mov	b,m
	inx	h
	mov	a,m
	sta	rep
	inx	h
	mov	a,b
	ora	a
	jz	nocopy
	xchg
	lhld	faddr
	xchg
cp:	mov	a,m
	stax	d
	inx	h
	inx	d
	dcr	b
	jnz	cp
nocopy:	shld	tp
again:	; fill the dma buffer with a pattern
	lda	cnt
	mvi	b,128
	lxi	h,80h
fl:	mov	m,a
	inr	a
	inx	h
	dcr	b
	jnz	fl
	lda	cnt
	inr	a
	sta	cnt
	; the call
	lda	func
	mov	c,a
	lhld	faddr
	xchg
	call	5
	sta	ra
	mov	a,b
	sta	rb
	shld	rhl
	; checksum of the dma buffer
	xra	a
	lxi	h,80h
	mvi	b,128
cs:	add	m
	inx	h
	dcr	b
	jnz	cs
	sta	rcs
	; report
	lda	func
	call	phex
	lda	ra
	call	phex
	lda	rb
	call	phex
	lda	rhl+1
	call	phex
	lda	rhl
	call	phex
	lda	rcs
	call	phex
	lda	dump
	ora	a
	jz	eol
	call	spc
	lhld	faddr
	mvi	b,36
dl:	mov	a,m
	push	h
	push	b
	call	phex
	pop	b
	pop	h
	inx	h
	dcr	b
	jnz	dl
eol:	mvi	e,13
	mvi	c,2
	call	5
	mvi	e,10
	mvi	c,2
	call	5
	lda	rep
	dcr	a
	sta	rep
	jnz	again
	jmp	next
done:	jmp	0

spc:	mvi	e,' '
	mvi	c,2
	call	5
	ret

phex:	push	psw
	rrc
	rrc
	rrc
	rrc
	call	nib
	pop	psw
	call	nib
	mvi	e,' '
	mvi	c,2
	call	5
	ret
nib:	ani	0fh
	adi	90h
	daa
	aci	40h
	daa
	mov	e,a
	mvi	c,2
	call	5
	ret

tp:	ds	2
faddr:	ds	2
func:	ds	1
dump:	ds	1
cnt:	ds	1
rep:	ds	1
ra:	ds	1
rb:	ds	1
rhl:	ds	2
rcs:	ds	1
	ds	64
stk:
table:
