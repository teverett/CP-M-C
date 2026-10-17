; JMPCCP.COM - exits by jumping to the CCP entry point (C = user/disk byte)
	org	100h
	mvi	e,'J'
	mvi	c,2
	call	5
	lda	4
	mov	c,a
	lxi	h,0dc00h
	pchl
