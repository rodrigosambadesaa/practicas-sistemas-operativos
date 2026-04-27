	.file	"cpu_stress.c"
	.text
	.section .rdata,"dr"
	.align 8
.LC2:
	.ascii "Iniciando carga de CPU... iteraciones=%llu\12\0"
	.align 8
.LC8:
	.ascii "Progreso: %llu / %llu | acc=%f\12\0"
	.align 8
.LC10:
	.ascii "Finalizado. Tiempo CPU aproximado: %.2f s | acc final=%f\12\0"
	.section	.text.startup,"x"
	.p2align 4
	.globl	main
	.def	main;	.scl	2;	.type	32;	.endef
	.seh_proc	main
main:
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$192, %rsp
	.seh_stackalloc	192
	movups	%xmm6, 48(%rsp)
	.seh_savexmm	%xmm6, 48
	movups	%xmm7, 64(%rsp)
	.seh_savexmm	%xmm7, 64
	movups	%xmm8, 80(%rsp)
	.seh_savexmm	%xmm8, 80
	movups	%xmm9, 96(%rsp)
	.seh_savexmm	%xmm9, 96
	movups	%xmm10, 112(%rsp)
	.seh_savexmm	%xmm10, 112
	movups	%xmm11, 128(%rsp)
	.seh_savexmm	%xmm11, 128
	movups	%xmm12, 144(%rsp)
	.seh_savexmm	%xmm12, 144
	movups	%xmm13, 160(%rsp)
	.seh_savexmm	%xmm13, 160
	movups	%xmm14, 176(%rsp)
	.seh_savexmm	%xmm14, 176
	.seh_endprologue
	movl	%ecx, %ebx
	movq	%rdx, %rbp
	call	__main
	cmpl	$1, %ebx
	jg	.L2
	movsd	.LC1(%rip), %xmm7
	movl	$300000000, %edi
	movl	$50000000, %esi
	movq	$0x000000000, 32(%rsp)
	movsd	%xmm7, 40(%rsp)
	call	clock
	movl	$300000000, %edx
	leaq	.LC2(%rip), %rcx
	movl	%eax, %ebp
	call	printf
.L3:
	movsd	.LC3(%rip), %xmm12
	movl	%esi, %esi
	movsd	.LC4(%rip), %xmm11
	movl	$1, %ebx
	movsd	.LC5(%rip), %xmm10
	movq	__imp___acrt_iob_func(%rip), %r12
	pxor	%xmm13, %xmm13
	movsd	.LC6(%rip), %xmm9
	movsd	.LC7(%rip), %xmm8
	jmp	.L12
	.p2align 4,,10
	.p2align 3
.L23:
	pxor	%xmm6, %xmm6
	cvtsi2sdq	%rbx, %xmm6
.L7:
	movsd	40(%rsp), %xmm0
	ucomisd	%xmm6, %xmm13
	mulsd	%xmm12, %xmm0
	addsd	%xmm11, %xmm0
	movsd	%xmm0, 40(%rsp)
	movapd	%xmm6, %xmm0
	ja	.L20
	sqrtsd	%xmm0, %xmm0
.L10:
	mulsd	%xmm7, %xmm0
	movsd	32(%rsp), %xmm1
	addsd	%xmm1, %xmm0
	movsd	%xmm0, 32(%rsp)
	movsd	40(%rsp), %xmm0
	call	sin
	movapd	%xmm0, %xmm14
	movapd	%xmm6, %xmm0
	addsd	%xmm9, %xmm6
	mulsd	%xmm10, %xmm0
	call	cos
	movapd	%xmm0, %xmm1
	movsd	32(%rsp), %xmm0
	mulsd	%xmm1, %xmm14
	addsd	%xmm0, %xmm14
	movapd	%xmm6, %xmm0
	movsd	%xmm14, 32(%rsp)
	call	log
	movq	%rbx, %rax
	xorl	%edx, %edx
	movsd	32(%rsp), %xmm1
	divq	%rsi
	mulsd	%xmm8, %xmm0
	subsd	%xmm0, %xmm1
	movsd	%xmm1, 32(%rsp)
	testq	%rdx, %rdx
	je	.L22
	addq	$1, %rbx
	cmpq	%rbx, %rdi
	jb	.L5
.L12:
	testq	%rbx, %rbx
	jns	.L23
	movq	%rbx, %rax
	movq	%rbx, %rdx
	pxor	%xmm6, %xmm6
	shrq	%rax
	andl	$1, %edx
	orq	%rdx, %rax
	cvtsi2sdq	%rax, %xmm6
	addsd	%xmm6, %xmm6
	jmp	.L7
	.p2align 4,,10
	.p2align 3
.L22:
	movq	32(%rsp), %r9
	movq	%rbx, %rdx
	movq	%rdi, %r8
	leaq	.LC8(%rip), %rcx
	addq	$1, %rbx
	movq	%r9, %xmm3
	call	printf
	movl	$1, %ecx
	call	*%r12
	movq	%rax, %rcx
	call	fflush
	cmpq	%rbx, %rdi
	jnb	.L12
.L5:
	call	clock
	movq	32(%rsp), %r8
	pxor	%xmm1, %xmm1
	leaq	.LC10(%rip), %rcx
	subl	%ebp, %eax
	movq	%r8, %xmm2
	cvtsi2sdl	%eax, %xmm1
	divsd	.LC9(%rip), %xmm1
	movq	%xmm1, %rdx
	call	printf
	nop
	movups	48(%rsp), %xmm6
	movups	64(%rsp), %xmm7
	xorl	%eax, %eax
	movups	80(%rsp), %xmm8
	movups	96(%rsp), %xmm9
	movups	128(%rsp), %xmm11
	movups	112(%rsp), %xmm10
	movups	144(%rsp), %xmm12
	movups	160(%rsp), %xmm13
	movups	176(%rsp), %xmm14
	addq	$192, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	ret
.L2:
	movq	8(%rbp), %rcx
	movl	$10, %r8d
	xorl	%edx, %edx
	movl	$50000000, %esi
	call	strtoull
	movq	%rax, %rdi
	cmpl	$2, %ebx
	je	.L4
	movq	16(%rbp), %rcx
	xorl	%edx, %edx
	movl	$10, %r8d
	movl	$1, %esi
	call	strtoul
	testl	%eax, %eax
	cmovne	%eax, %esi
.L4:
	movsd	.LC1(%rip), %xmm7
	movq	$0x000000000, 32(%rsp)
	movsd	%xmm7, 40(%rsp)
	call	clock
	movq	%rdi, %rdx
	leaq	.LC2(%rip), %rcx
	movl	%eax, %ebp
	call	printf
	testq	%rdi, %rdi
	jne	.L3
	jmp	.L5
.L20:
	call	sqrt
	jmp	.L10
	.seh_endproc
	.section .rdata,"dr"
	.align 8
.LC1:
	.long	-1598689907
	.long	1051772663
	.align 8
.LC3:
	.long	450359963
	.long	1072693248
	.align 8
.LC4:
	.long	-200440970
	.long	1049895413
	.align 8
.LC5:
	.long	-1698910392
	.long	1048238066
	.align 8
.LC6:
	.long	0
	.long	1072693248
	.align 8
.LC7:
	.long	-500134854
	.long	1044740494
	.align 8
.LC9:
	.long	0
	.long	1083129856
	.def	__main;	.scl	2;	.type	32;	.endef
	.ident	"GCC: (Rev8, Built by MSYS2 project) 15.2.0"
	.def	clock;	.scl	2;	.type	32;	.endef
	.def	printf;	.scl	2;	.type	32;	.endef
	.def	sin;	.scl	2;	.type	32;	.endef
	.def	cos;	.scl	2;	.type	32;	.endef
	.def	log;	.scl	2;	.type	32;	.endef
	.def	fflush;	.scl	2;	.type	32;	.endef
	.def	strtoull;	.scl	2;	.type	32;	.endef
	.def	strtoul;	.scl	2;	.type	32;	.endef
	.def	sqrt;	.scl	2;	.type	32;	.endef
