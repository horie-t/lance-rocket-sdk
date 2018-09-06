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
	
	# mepcを復元
	lw t0, 16*REGBYTES(sp)
	csrw mepc, t0
	
	# 呼び出し側に保存責任があるレジスタを復元	
	lw x1,  0*REGBYTES(sp)
	lw x5,  1*REGBYTES(sp)
	lw x6,  2*REGBYTES(sp)
	lw x7,  3*REGBYTES(sp)
	lw x10, 4*REGBYTES(sp)
	lw x11, 5*REGBYTES(sp)
	lw x12, 6*REGBYTES(sp)
	lw x13, 7*REGBYTES(sp)
	lw x14, 8*REGBYTES(sp)
	lw x15, 9*REGBYTES(sp)
	lw x16, 10*REGBYTES(sp)
	lw x17, 11*REGBYTES(sp)
	lw x28, 12*REGBYTES(sp)
	lw x29, 13*REGBYTES(sp)
	lw x30, 14*REGBYTES(sp)
	lw x31, 15*REGBYTES(sp)
	
	addi sp, sp, 17*REGBYTES
	
	mret
