	.file	"memory_stress.c"
	.text
	.section .rdata,"dr"
.LC0:
	.ascii "definido\0"
.LC1:
	.ascii "sin limite\0"
	.align 8
.LC2:
	.ascii "Iniciando consumo progresivo de memoria...\0"
	.align 8
.LC3:
	.ascii "Error: MB por paso debe ser > 0.\12\0"
	.align 8
.LC4:
	.ascii "Paso: %zu MB | Sleep: %u s | Max pasos: %s\12\0"
	.align 8
.LC5:
	.ascii "realloc fallo tras reservar %zu MB totales.\12\0"
	.align 8
.LC6:
	.ascii "malloc fallo en el paso %zu. Reservado: %zu MB\12\0"
	.align 8
.LC7:
	.ascii "Paso %zu: reservado total %zu MB | fisica disponible aprox %llu MB\12\0"
	.align 8
.LC8:
	.ascii "Paso %zu: reservado total %zu MB\12\0"
.LC9:
	.ascii "Liberando memoria...\0"
.LC10:
	.ascii "Fin.\0"
	.section	.text.startup,"x"
	.p2align 4
	.globl	main
	.def	main;	.scl	2;	.type	32;	.endef
	.seh_proc	main
main:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
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
	subq	$136, %rsp
	.seh_stackalloc	136
	.seh_endprologue
	movl	%ecx, %ebx
	movq	%rdx, %rsi
	call	__main
	cmpl	$1, %ebx
	jg	.L2
	leaq	.LC2(%rip), %rcx
	movl	$67108864, %r12d
	xorl	%ebx, %ebx
	movl	$64, %r14d
	call	puts
	movl	$1, %edi
	leaq	.LC1(%rip), %r9
.L3:
	movl	%edi, %r8d
	movq	%r14, %rdx
	leaq	.LC4(%rip), %rcx
	xorl	%ebp, %ebp
	call	printf
	leaq	-1(%rbx), %rax
	xorl	%esi, %esi
	xorl	%ebx, %ebx
	movq	%rax, 48(%rsp)
	imull	$1000, %edi, %eax
	movq	__imp___acrt_iob_func(%rip), %r15
	movl	%eax, 44(%rsp)
	leaq	64(%rsp), %rax
	movq	%rax, 56(%rsp)
	jmp	.L15
	.p2align 4,,10
	.p2align 3
.L40:
	movq	80(%rsp), %r9
	shrq	$20, %r9
	je	.L13
	leaq	.LC7(%rip), %rcx
	call	printf
.L14:
	movl	$1, %ecx
	call	*%r15
	movq	%rax, %rcx
	call	fflush
	movl	44(%rsp), %ecx
	call	*__imp_Sleep(%rip)
	cmpq	%rsi, 48(%rsp)
	jb	.L37
.L15:
	movq	%rsi, %rdi
	leaq	1(%rsi), %rsi
	movq	%rbx, %rcx
	movq	%rbx, %r13
	leaq	0(,%rsi,8), %rdx
	call	realloc
	movq	%rax, %rbx
	testq	%rax, %rax
	je	.L38
	movq	%r12, %rcx
	call	malloc
	movq	%rax, (%rbx,%rdi,8)
	testq	%rax, %rax
	je	.L39
	movq	%r12, %r8
	movl	$165, %edx
	movq	%rax, %rcx
	addq	%r14, %rbp
	call	memset
	movl	$64, 64(%rsp)
	movq	56(%rsp), %rcx
	call	*__imp_GlobalMemoryStatusEx(%rip)
	movq	%rbp, %r8
	movq	%rsi, %rdx
	testl	%eax, %eax
	jne	.L40
.L13:
	leaq	.LC8(%rip), %rcx
	call	printf
	jmp	.L14
.L37:
	leaq	.LC9(%rip), %rcx
	movq	%rsi, %rdi
	call	puts
.L19:
	movq	%rbx, %rsi
	leaq	(%rbx,%rdi,8), %rdi
	.p2align 4
	.p2align 3
.L18:
	movq	(%rsi), %rcx
	addq	$8, %rsi
	call	free
	cmpq	%rdi, %rsi
	jne	.L18
.L17:
	movq	%rbx, %rcx
	call	free
	leaq	.LC10(%rip), %rcx
	call	puts
	xorl	%eax, %eax
.L1:
	addq	$136, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.L2:
	movq	8(%rsi), %rcx
	movl	$10, %r8d
	xorl	%edx, %edx
	call	strtoull
	movq	%rax, %r14
	cmpl	$2, %ebx
	jne	.L4
	testq	%rax, %rax
	je	.L8
	movq	%rax, %r12
	leaq	.LC2(%rip), %rcx
	xorl	%ebx, %ebx
	movl	$1, %edi
	call	puts
	salq	$20, %r12
	leaq	.LC1(%rip), %r9
	jmp	.L3
.L4:
	movq	16(%rsi), %rcx
	movl	$10, %r8d
	xorl	%edx, %edx
	call	strtoul
	movl	%eax, %edi
	cmpl	$3, %ebx
	je	.L41
	movq	24(%rsi), %rcx
	movl	$10, %r8d
	xorl	%edx, %edx
	call	strtoull
	movq	%rax, %rbx
	testq	%r14, %r14
	jne	.L42
.L8:
	movl	$2, %ecx
	call	*__imp___acrt_iob_func(%rip)
	movl	$33, %r8d
	movl	$1, %edx
	leaq	.LC3(%rip), %rcx
	movq	%rax, %r9
	call	fwrite
	movl	$1, %eax
	jmp	.L1
.L42:
	movq	%r14, %r12
	leaq	.LC2(%rip), %rcx
	salq	$20, %r12
	call	puts
	testq	%rbx, %rbx
	je	.L20
	leaq	.LC0(%rip), %r9
	jmp	.L3
.L41:
	testq	%r14, %r14
	je	.L8
	movq	%r14, %r12
	leaq	.LC2(%rip), %rcx
	xorl	%ebx, %ebx
	call	puts
	salq	$20, %r12
	leaq	.LC1(%rip), %r9
	jmp	.L3
.L39:
	movl	$2, %ecx
	call	*%r15
	movq	%rbp, %r9
	movq	%rsi, %r8
	leaq	.LC6(%rip), %rdx
	movq	%rax, %rcx
	call	fprintf
.L10:
	leaq	.LC9(%rip), %rcx
	call	puts
	testq	%rdi, %rdi
	je	.L17
	jmp	.L19
.L38:
	movl	$2, %ecx
	movq	%r13, %rbx
	call	*%r15
	movq	%rbp, %r8
	leaq	.LC5(%rip), %rdx
	movq	%rax, %rcx
	call	fprintf
	jmp	.L10
.L20:
	leaq	.LC1(%rip), %r9
	jmp	.L3
	.seh_endproc
	.def	__main;	.scl	2;	.type	32;	.endef
	.ident	"GCC: (Rev8, Built by MSYS2 project) 15.2.0"
	.def	puts;	.scl	2;	.type	32;	.endef
	.def	printf;	.scl	2;	.type	32;	.endef
	.def	fflush;	.scl	2;	.type	32;	.endef
	.def	realloc;	.scl	2;	.type	32;	.endef
	.def	malloc;	.scl	2;	.type	32;	.endef
	.def	memset;	.scl	2;	.type	32;	.endef
	.def	free;	.scl	2;	.type	32;	.endef
	.def	strtoull;	.scl	2;	.type	32;	.endef
	.def	strtoul;	.scl	2;	.type	32;	.endef
	.def	fwrite;	.scl	2;	.type	32;	.endef
	.def	fprintf;	.scl	2;	.type	32;	.endef
