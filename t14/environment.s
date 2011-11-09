	.file	"environment.c"
	.section	.rodata
.LC0:
	.string	"%s\n"
	.text
.globl main
	.type	main, @function
main:
	pushl	%ebp
	movl	%esp, %ebp
	andl	$-16, %esp
	subl	$32, %esp
	movl	environ, %eax
	movl	%eax, 24(%esp)
	movl	$0, 28(%esp)
	jmp	.L2
.L3:
	movl	28(%esp), %eax
	sall	$2, %eax
	addl	12(%ebp), %eax
	movl	(%eax), %ecx
	movl	$.LC0, %edx
	movl	stdout, %eax
	movl	%ecx, 8(%esp)
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	fprintf
	addl	$1, 28(%esp)
.L2:
	movl	28(%esp), %eax
	cmpl	8(%ebp), %eax
	jl	.L3
	jmp	.L4
.L5:
	movl	24(%esp), %eax
	movl	(%eax), %ecx
	addl	$4, 24(%esp)
	movl	$.LC0, %edx
	movl	stdout, %eax
	movl	%ecx, 8(%esp)
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	fprintf
.L4:
	movl	24(%esp), %eax
	movl	(%eax), %eax
	testl	%eax, %eax
	jne	.L5
	movl	$0, %eax
	leave
	ret
	.size	main, .-main
	.ident	"GCC: (Ubuntu 4.4.3-4ubuntu5) 4.4.3"
	.section	.note.GNU-stack,"",@progbits
