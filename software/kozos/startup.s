# See LICENSE.HORIE_Tetsuya for license details.

	.equ REGBYTES, 4
	
	.section .text.init
	.option norvc
	.global _start
_start:
	la sp, bootstack
	call main
1:
	j 1b 	# 無限ループ

	.global dispatch
dispatch:
	lw sp, 0(a0)
	
	# 割り込まれたPC, mstatus.mieを復元
	lw t0, 33*REGBYTES(sp)
	csrw mepc, t0
#	lw t0, 32*REGBYTES(sp)
#	csrs mstatus, t0
	
	# 呼び出し側に保存責任があるレジスタを復元	
	lw x1, 1*REGBYTES(sp)
#	lw x2, 2*REGBYTES(sp)
	lw x3, 3*REGBYTES(sp)
	lw x4, 4*REGBYTES(sp)
	lw x5, 5*REGBYTES(sp)
	lw x6, 6*REGBYTES(sp)
	lw x7, 7*REGBYTES(sp)
	lw x8, 8*REGBYTES(sp)
	lw x9, 9*REGBYTES(sp)
	lw x10, 10*REGBYTES(sp)
	lw x11, 11*REGBYTES(sp)
	lw x12, 12*REGBYTES(sp)
	lw x13, 13*REGBYTES(sp)
	lw x14, 14*REGBYTES(sp)
	lw x15, 15*REGBYTES(sp)
	lw x16, 16*REGBYTES(sp)
	lw x17, 17*REGBYTES(sp)
	lw x18, 18*REGBYTES(sp)
	lw x19, 19*REGBYTES(sp)
	lw x20, 20*REGBYTES(sp)
	lw x21, 21*REGBYTES(sp)
	lw x22, 22*REGBYTES(sp)
	lw x23, 23*REGBYTES(sp)
	lw x24, 24*REGBYTES(sp)
	lw x25, 25*REGBYTES(sp)
	lw x26, 26*REGBYTES(sp)
	lw x27, 27*REGBYTES(sp)
	lw x28, 28*REGBYTES(sp)
	lw x29, 29*REGBYTES(sp)
	lw x30, 30*REGBYTES(sp)
	lw x31, 31*REGBYTES(sp)

	addi sp, sp, 34*REGBYTES
	
	mret
