; load values
	LOD	r1, v1
	LOD	r2, v2
	LOD	r3, v3

; print values
	WRIT	1, r1
	WRIT	1, r2
	WRIT	1, r3

; end
	HALT

v1:	WORD	0x48454c4c
v2:	WORD	0x4f20574f
v3:	WORD	0x524c440a
