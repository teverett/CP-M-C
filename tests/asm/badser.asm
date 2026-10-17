; BADSER.COM - scribbles on the BDOS serial number block, then returns
	org	100h
	mvi	a,1
	sta	0e400h
	ret
