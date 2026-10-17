; JMPCCP2.COM - exits by jumping to ccpclear (CCP base + 3)
	org	100h
	mvi	e,'K'
	mvi	c,2
	call	5
	lda	4
	mov	c,a
	lxi	h,0dc03h
	pchl
