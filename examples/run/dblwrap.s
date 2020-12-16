# COMM wrap-1.0 (C)prs, 2020 generated at Thu Dec 10 15:08:09 2020

# GLOBL
.globl	_readr	# :function
# LABEL
_readr:
# EXTRN
.extern	readr
# CALL
	call	readr
# RET
	ret
# GLOBL
.globl	_printr	# :function
# LABEL
_printr:
	movq	8(%rsp), %rdi
# EXTRN
.extern	printr
# CALL
	call	printr
# RET
	ret
# GLOBL
.globl	_readd	# :function
# LABEL
_readd:
# EXTRN
.extern	readd
# CALL
	call	readd
# RET
	ret
# GLOBL
.globl	_printd	# :function
# LABEL
_printd:
	movq	8(%rsp), %xmm0
# EXTRN
.extern	printd
# CALL
	call	printd
# RET
	ret
# GLOBL
.globl	_atod	# :function
# LABEL
_atod:
	movq	8(%rsp), %rdi
# EXTRN
.extern	atod
# CALL
	call	atod
# RET
	ret
# GLOBL
.globl	_dtoa	# :function
# LABEL
_dtoa:
	movq	8(%rsp), %rdi
	movq	16(%rsp), %rsi
	movq	24(%rsp), %xmm0
# EXTRN
.extern	dtoa
# CALL
	call	dtoa
# RET
	ret
# GLOBL
.globl	_atof	# :function
# LABEL
_atof:
	movq	8(%rsp), %rdi
# EXTRN
.extern	atof
# CALL
	call	atof
# RET
	ret
# GLOBL
.globl	_ftoa	# :function
# LABEL
_ftoa:
	movq	8(%rsp), %rdi
	movq	16(%rsp), %rsi
	movq	24(%rsp), %xmm0
# EXTRN
.extern	ftoa
# CALL
	call	ftoa
# RET
	ret
