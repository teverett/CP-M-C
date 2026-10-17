; JMPCCP2.COM - exits by jumping to ccpclear (CCP base + 3)
	org	100h
	mvi	e,'K'
	mvi	c,2
	call	5
	lda	4
	mov	c,a
	lhld	6		; BDOS entry; the CCP is 800h below the BDOS
	mov	a,h
	sui	8
	mov	h,a
	mvi	l,3
	pchl
