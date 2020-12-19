# COMM wrap-1.0 (C)prs, 2020 generated at Sat Dec 19 15:04:46 2020

# GLOBL
.globl	_char	# :function
# LABEL
_char:
	movq	8(%rsp), %rdi
	movq	16(%rsp), %rsi
# EXTRN
.extern	Char
# CALL
	call	Char
# RET
	ret
# GLOBL
.globl	_lchar	# :function
# LABEL
_lchar:
	movq	8(%rsp), %rdi
	movq	16(%rsp), %rsi
	movq	24(%rsp), %rdx
# EXTRN
.extern	lchar
# CALL
	call	lchar
# RET
	ret
# GLOBL
.globl	_putchar	# :function
# LABEL
_putchar:
	movq	8(%rsp), %rdi
# EXTRN
.extern	putchar
# CALL
	call	putchar
# RET
	ret
