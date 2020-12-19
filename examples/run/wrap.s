# COMM wrap-1.0 (C)prs, 2020 generated at Sat Dec 19 15:07:50 2020

# GLOBL
.globl	atoi	# :function
# LABEL
atoi:
	pushq	%rdi
# EXTRN
.extern	_atoi
# CALL
	call	_atoi
# TRASH
	add	$8, %rsp
# RET
	ret
# GLOBL
.globl	prints	# :function
# LABEL
prints:
	pushq	%rdi
# EXTRN
.extern	_prints
# CALL
	call	_prints
# TRASH
	add	$8, %rsp
# RET
	ret
# GLOBL
.globl	readln	# :function
# LABEL
readln:
	pushq	%rsi
	pushq	%rdi
# EXTRN
.extern	_readln
# CALL
	call	_readln
# TRASH
	add	$16, %rsp
# RET
	ret
# GLOBL
.globl	println	# :function
# LABEL
println:
# EXTRN
.extern	_println
# CALL
	call	_println
# RET
	ret
# GLOBL
.globl	printsp	# :function
# LABEL
printsp:
	pushq	%rdi
# EXTRN
.extern	_printsp
# CALL
	call	_printsp
# TRASH
	add	$8, %rsp
# RET
	ret
# GLOBL
.globl	readb	# :function
# LABEL
readb:
# EXTRN
.extern	_readb
# CALL
	call	_readb
# RET
	ret
# GLOBL
.globl	argc	# :function
# LABEL
argc:
# EXTRN
.extern	_argc
# CALL
	call	_argc
# RET
	ret
# GLOBL
.globl	argv	# :function
# LABEL
argv:
	pushq	%rdi
# EXTRN
.extern	_argv
# CALL
	call	_argv
# TRASH
	add	$8, %rsp
# RET
	ret
# GLOBL
.globl	envp	# :function
# LABEL
envp:
	pushq	%rdi
# EXTRN
.extern	_envp
# CALL
	call	_envp
# TRASH
	add	$8, %rsp
# RET
	ret
# GLOBL
.globl	itoa	# :function
# LABEL
itoa:
	pushq	%rdi
# EXTRN
.extern	_itoa
# CALL
	call	_itoa
# TRASH
	add	$8, %rsp
# RET
	ret
# GLOBL
.globl	strlen	# :function
# LABEL
strlen:
	pushq	%rdi
# EXTRN
.extern	_strlen
# CALL
	call	_strlen
# TRASH
	add	$8, %rsp
# RET
	ret
# GLOBL
.globl	printi	# :function
# LABEL
printi:
	pushq	%rdi
# EXTRN
.extern	_printi
# CALL
	call	_printi
# TRASH
	add	$8, %rsp
# RET
	ret
# GLOBL
.globl	readi	# :function
# LABEL
readi:
# EXTRN
.extern	_readi
# CALL
	call	_readi
# RET
	ret
# GLOBL
.globl	_putstr	# :function
# LABEL
_putstr:
	movq	8(%rsp), %rdi
# EXTRN
.extern	prints
# CALL
	call	prints
# RET
	ret
# GLOBL
.globl	_putint	# :function
# LABEL
_putint:
	movq	8(%rsp), %rdi
# EXTRN
.extern	printi
# CALL
	call	printi
# RET
	ret
# GLOBL
.globl	_getint	# :function
# LABEL
_getint:
# EXTRN
.extern	readi
# CALL
	call	readi
# RET
	ret
# GLOBL
.globl	_getstr	# :function
# LABEL
_getstr:
	movq	8(%rsp), %rdi
	movq	16(%rsp), %rsi
# EXTRN
.extern	readln
# CALL
	call	readln
# RET
	ret
# GLOBL
.globl	_getchar	# :function
# LABEL
_getchar:
# EXTRN
.extern	readb
# CALL
	call	readb
# RET
	ret
