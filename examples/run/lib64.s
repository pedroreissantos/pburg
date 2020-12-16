.set	BUFsiz,	32
.section .rodata
nl:	.byte	10, 0
sp:	.byte	32, 0
.section .data
.globl _env
_env:	.quad	0
bufsiz:	.quad	BUFsiz
.section .bss
buffer:	.space	BUFsiz

.section .text
.globl _println, _printsp, _printi, _readln, _readi, _debug
.globl _strlen, _atoi, _itoa, _argc, _argv, _envp
.extern _prints, _readb

_argc:	movq	(_env), %rax
	movq	(%rax), %rax
	ret

_argv:	movq	(_env), %rax
	addq	$8, %rax
	movq	8(%rsp), %rbx
	leaq	(%rax,%rbx,8), %rax
	movq	(%rax), %rax
	ret

_envp:	movq	(_env), %rax
	movq	%rax, %rbx
	movq	(%rbx), %rdx
	leaq	16(%rbx,%rdx,8), %rax
	movq	8(%rsp), %rbx
	leaq	(%rax,%rbx,8), %rax
	movq	(%rax), %rax
	ret

_strlen:
	movq	8(%rsp), %rdx
	cmpb	$0, (%rdx)
	movq	%rdx, %rax
	je	.Lend
.Lrep:	incq	%rax
	cmpb	$0, (%rax)
	jne	.Lrep
.Lend:	subq	%rdx, %rax
	ret

_println:
	leaq	nl(%rip), %rax
#	mov	nl, %rax
	pushq	%rax
	call	_prints
	addq	$8, %rsp
	ret

_printsp:
	leaq	sp(%rip), %rax
#	mov	sp, %rax
	pushq	%rax
.Lmais:	# cmp	qword [%rsp+16], 0
	movq	16(%rsp), %rax
	cmpq	$0, %rax
	jle	.Lfim
	call	_prints
	decq	16(%rsp)
	jmp	.Lmais
.Lfim:	addq	$8, %rsp
	ret

_debug:	ret

_itoa:
	movq	8(%rsp), %rcx	# load arg
	leaq	buffer(%rip), %rdi
	addq	$30, %rdi
#	movq	buffer+30, %rdi
	movq	$0, %rsi
	testq	%rcx, %rcx
	jge	.L8
	incq	%rsi
	negq	%rcx
.L8:	xorq	%rax, %rax
	movb	%al, (%rdi)	# put the NULL character and walk backwards
	decq	%rdi
	movq	$10, %rbx
.L9:	movq	%rcx, %rax
	xorq	%rdx, %rdx
	divq	%rbx
	movq	%rax, %rcx
	addq	$'0', %rdx
	movb	%dl, (%rdi)
	decq	%rdi
	testq	%rcx, %rcx
	jg	.L9
	testq	%rsi, %rsi
	je	.L10
	movb	$'-', (%rdi)
	decq	%rdi
.L10:	movq	%rdi, %rax
	incq	%rax
	ret

_printi:
	pushq	8(%rsp)
	call	_itoa
	addq	$8, %rsp
	pushq	%rax
	call	_prints
	addq	$8, %rsp
	ret

_atoi:
	movq	8(%rsp), %rsi
	movq	$1, %rcx
	xorq	%rbx, %rbx
	xorq	%rax, %rax
	cld
	lodsb
	cmpq	$'-', %rax
	jne	.L1
	negq	%rcx
	lodsb
.L1:	cmpq	$'0', %rax
	jb	.L2
	cmpq	$'9', %rax
	ja	.L2
	subq	$'0', %rax
	xchgq	%rax, %rbx
	movq	$10, %rdx
	mulq	%rdx
	addq	%rax, %rbx
	xorq	%rax, %rax
	lodsb
	jmp	.L1
.L2:	movq	%rbx, %rax
	mulq	%rcx
	ret

_readln:			#  readln(buf, siz)
	movq	16(%rsp), %rcx
	movq	8(%rsp), %rdi
	cld
_Lnext:	call	_readb
	cmpq	$0, %rax
	jle	_Lerr
	cmpq	$10, %rax
	je	_Lend
	cmpq	$1, %rcx
	jle	_Lend
	stosb
	decq	%rcx
	jmp	_Lnext
_Lend:	xorq	%rax, %rax
	stosb
	movq	8(%rsp), %rax
	ret
_Lerr:	xorq	%rax, %rax
	ret

_readi:
	leaq	bufsiz(%rip), %rax
#	mov	bufsiz, %rax
	pushq	%rax
	leaq	buffer(%rip), %rax
#	mov	buffer, %rax
	pushq	%rax
	call	_readln
	addq	$16, %rsp
	cmpq	$0, %rax
	je	.Lerr
	pushq	%rax
	call	_atoi
	addq	$8, %rsp
	ret
.Lerr:	movq	$0x8000000000000000, %rax
	ret
