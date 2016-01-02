	LOD r1, v
	BRAN l

v:	WORD 0x11

l:	HALT
	BRGE l
	STO r1, v
