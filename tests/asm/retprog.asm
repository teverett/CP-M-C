; RETPROG.COM - uses the CCP's stack and returns with RET
	org	100h
	mvi	e,'R'
	mvi	c,2
	call	5
	mvi	e,13
	mvi	c,2
	call	5
	mvi	e,10
	mvi	c,2
	call	5
	ret
