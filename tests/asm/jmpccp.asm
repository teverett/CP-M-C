; JMPCCP.COM - exits by jumping to the CCP entry point (C = user/disk byte)
	org	100h
	mvi	e,'J'
	mvi	c,2
	call	5
	lda	4
	mov	c,a
	lhld	6		; BDOS entry; the CCP is 800h below the BDOS
	mov	a,h
	sui	8
	mov	h,a
	mvi	l,0
	pchl
