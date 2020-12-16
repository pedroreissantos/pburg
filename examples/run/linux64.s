.section .bss
.align 8
buf2:	.space	2

.section .text
.globl _prints, _readb, _exit, _start
.extern _env, _strlen, _main

_prints:
	pushq	8(%rsp)		# pushd 'prints' first argument
	call	_strlen
	movq	%rax, %rdx	# strlen
	popq	%rsi		# string
	movq	$1, %rdi	# stdout
	movq	$1, %rax	# SYS_write
	syscall
	ret

_readb:
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	pushq	%rcx
	movq	$1, %rdx		# bytes
	leaq	buf2(%rip), %rsi	# buffer
#	movq	buf2, %rsi		# buffer
	movq	$0, %rdi		# stdin
	movq	$0, %rax		# SYS_read
	syscall
	cmpq	$1, %rax
	jne	.Lret
	xorq	%rax, %rax
	mov	(buf2), %al
.Lret:	popq	%rcx
	popq	%rdx
	popq	%rsi
	popq	%rdi
	ret

_start:	
	movq	%rsp, (_env)
	movq	(%rsp), %rdi		# argc
	leaq	8(%rsp), %rsi		# argv
	leaq	8(%rsi,%rdi,8), %rdx	# envp
	pushq	$0
	movq	%rsp, %rbp	# init frame pointer
	pushq	%rdx
	pushq	%rsi
	pushq	%rdi
#	call	__alloca	# glibc
#	call	___main		# glibc
	call	_main
	movq	%rax, %rdi
	pushq	%rax
	call	__exit

__exit:	movq	8(%rsp), %rbx
.L0:	movq	$60, %rax
	syscall
	jmp	.L0
