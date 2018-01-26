	.file	"server_test.cpp"
	.section	.rodata
.LC0:
	.string	"2"
.LC1:
	.string	"/home/ubuntu/clientImg/"
.LC2:
	.string	"/home/ubuntu/timg2.jpg"
.LC3:
	.string	"./client"
.LC4:
	.string	"/home/ubuntu/server_v2/client"
.LC5:
	.string	"3"
.LC6:
	.string	"/home/ubuntu/timg4.jpg"
	.text
	.globl	main
	.type	main, @function
main:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$64, %rsp
	movl	%edi, -20(%rbp)
	movq	%rsi, -32(%rbp)
	movl	$50, -8(%rbp)
	movl	$0, -16(%rbp)
	jmp	.L2
.L7:
	call	fork
	movl	%eax, -4(%rbp)
	cmpl	$0, -4(%rbp)
	jne	.L3
	movl	$0, -12(%rbp)
	jmp	.L4
.L5:
	movq	$0, 24(%rsp)
	movq	$.LC5, 16(%rsp)
	movq	$.LC1, 8(%rsp)
	movq	$.LC6, (%rsp)
	movl	$.LC0, %r9d
	movl	$.LC1, %r8d
	movl	$.LC2, %ecx
	movl	$.LC0, %edx
	movl	$.LC3, %esi
	movl	$.LC4, %edi
	movl	$0, %eax
	call	execl
	addl	$1, -12(%rbp)
.L4:
	cmpl	$19, -12(%rbp)
	jle	.L5
	movl	$0, %eax
	jmp	.L6
.L3:
	addl	$1, -16(%rbp)
.L2:
	movl	-16(%rbp), %eax
	cmpl	-8(%rbp), %eax
	jl	.L7
	movl	$0, %eax
.L6:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4"
	.section	.note.GNU-stack,"",@progbits
