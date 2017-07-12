	.globl	asm_read128
	.type	asm_read128,%function

asm_read128:
	.fnstart
	vld1.64		{d0,d1},[r1]
	vstmia.64 	r0, {d0,d1}
	bx 			lr
	.fnend


	.globl	asm_write128
	.type 	asm_write128,%function
asm_write128:
	.fnstart
	vldmia.64	r0, {d0,d1}
	vst1.64		{d0,d1},[r1]
	bx			lr
	.fnend
