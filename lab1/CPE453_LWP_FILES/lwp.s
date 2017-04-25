	.file	"lwp.c"
	.globl	global_scheduler
	.bss
	.align 8
	.type	global_scheduler, @object
	.size	global_scheduler, 8
global_scheduler:
	.zero	8
	.comm	global_stack_pointer,8,8
	.comm	lwp_ptable,960,32
	.globl	lwp_procs
	.align 4
	.type	lwp_procs, @object
	.size	lwp_procs, 4
lwp_procs:
	.zero	4
	.comm	lwp_running,4,4
	.globl	next_pid
	.align 8
	.type	next_pid, @object
	.size	next_pid, 8
next_pid:
	.zero	8
	.text
	.globl	new_lwp
	.type	new_lwp, @function
new_lwp:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$56, %rsp
	.cfi_offset 3, -24
	movq	%rdi, -40(%rbp)
	movq	%rsi, -48(%rbp)
	movq	%rdx, -56(%rbp)
	movl	lwp_procs(%rip), %eax
	cmpl	$30, %eax
	jne	.L2
	movl	$-1, %eax
	jmp	.L3
.L2:
	movl	lwp_procs(%rip), %eax
	leal	1(%rax), %edx
	movl	%edx, lwp_procs(%rip)
	movl	%eax, lwp_running(%rip)
	movl	lwp_running(%rip), %ecx
	movq	next_pid(%rip), %rax
	leaq	1(%rax), %rdx
	movq	%rdx, next_pid(%rip)
	movslq	%ecx, %rdx
	salq	$5, %rdx
	addq	$lwp_ptable, %rdx
	movq	%rax, (%rdx)
	movl	lwp_running(%rip), %eax
	cltq
	salq	$5, %rax
	leaq	lwp_ptable+16(%rax), %rdx
	movq	-56(%rbp), %rax
	movq	%rax, (%rdx)
	movl	lwp_running(%rip), %ebx
	movq	-56(%rbp), %rax
	salq	$2, %rax
	movq	%rax, %rdi
	call	malloc
	movslq	%ebx, %rdx
	salq	$5, %rdx
	addq	$lwp_ptable, %rdx
	movq	%rax, 8(%rdx)
	movl	lwp_running(%rip), %eax
	cltq
	salq	$5, %rax
	addq	$lwp_ptable, %rax
	movq	8(%rax), %rax
	movq	-56(%rbp), %rdx
	salq	$5, %rdx
	addq	%rdx, %rax
	movq	%rax, -32(%rbp)
	movq	-32(%rbp), %rax
	movq	%rax, -24(%rbp)
	movq	-48(%rbp), %rdx
	movq	-32(%rbp), %rax
	movq	%rdx, (%rax)
	subq	$8, -32(%rbp)
	movl	$lwp_exit, %edx
	movq	-32(%rbp), %rax
	movq	%rdx, (%rax)
	subq	$8, -32(%rbp)
	movq	-40(%rbp), %rdx
	movq	-32(%rbp), %rax
	movq	%rdx, (%rax)
	subq	$8, -32(%rbp)
	movq	-32(%rbp), %rax
	movl	$4276993775, %esi
	movq	%rsi, (%rax)
	movq	-32(%rbp), %rax
	movq	%rax, -24(%rbp)
	subq	$56, -32(%rbp)
	movq	-24(%rbp), %rdx
	movq	-32(%rbp), %rax
	movq	%rdx, (%rax)
	movl	lwp_running(%rip), %eax
	cltq
	salq	$5, %rax
	leaq	lwp_ptable+16(%rax), %rdx
	movq	-32(%rbp), %rax
	movq	%rax, 8(%rdx)
	movl	lwp_running(%rip), %eax
.L3:
	addq	$56, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	new_lwp, .-new_lwp
	.globl	lwp_getpid
	.type	lwp_getpid, @function
lwp_getpid:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	lwp_running(%rip), %eax
	cltq
	salq	$5, %rax
	addq	$lwp_ptable, %rax
	movq	(%rax), %rax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	lwp_getpid, .-lwp_getpid
	.globl	lwp_yield
	.type	lwp_yield, @function
lwp_yield:
.LFB4:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
#APP
# 47 "lwp.c" 1
	pushq %rax
# 0 "" 2
# 47 "lwp.c" 1
	pushq %rbx
# 0 "" 2
# 47 "lwp.c" 1
	pushq %rcx
# 0 "" 2
# 47 "lwp.c" 1
	pushq %rdx
# 0 "" 2
# 47 "lwp.c" 1
	pushq %rsi
# 0 "" 2
# 47 "lwp.c" 1
	pushq %rdi
# 0 "" 2
# 47 "lwp.c" 1
	pushq %r8
# 0 "" 2
# 47 "lwp.c" 1
	pushq %r9
# 0 "" 2
# 47 "lwp.c" 1
	pushq %r10
# 0 "" 2
# 47 "lwp.c" 1
	pushq %r11
# 0 "" 2
# 47 "lwp.c" 1
	pushq %r12
# 0 "" 2
# 47 "lwp.c" 1
	pushq %r13
# 0 "" 2
# 47 "lwp.c" 1
	pushq %r14
# 0 "" 2
# 47 "lwp.c" 1
	pushq %r15
# 0 "" 2
# 47 "lwp.c" 1
	pushq %rbp
# 0 "" 2
#NO_APP
	movl	lwp_running(%rip), %edx
#APP
# 48 "lwp.c" 1
	movq  %rsp,%rax
# 0 "" 2
#NO_APP
	movslq	%edx, %rdx
	salq	$5, %rdx
	addq	$lwp_ptable+16, %rdx
	movq	%rax, 8(%rdx)
	movq	global_scheduler(%rip), %rax
	testq	%rax, %rax
	jne	.L7
	movl	lwp_procs(%rip), %eax
	leal	-1(%rax), %edx
	movl	lwp_running(%rip), %eax
	cmpl	%eax, %edx
	jne	.L8
	movl	$0, lwp_running(%rip)
	jmp	.L10
.L8:
	movl	lwp_running(%rip), %eax
	addl	$1, %eax
	movl	%eax, lwp_running(%rip)
	jmp	.L10
.L7:
	movq	global_scheduler(%rip), %rax
	call	*%rax
	movl	%eax, lwp_running(%rip)
.L10:
	movl	lwp_running(%rip), %eax
	cltq
	salq	$5, %rax
	addq	$lwp_ptable+16, %rax
	movq	8(%rax), %rax
#APP
# 58 "lwp.c" 1
	movq  %rax,%rsp
# 0 "" 2
# 60 "lwp.c" 1
	popq  %rbp
# 0 "" 2
# 60 "lwp.c" 1
	popq  %r15
# 0 "" 2
# 60 "lwp.c" 1
	popq  %r14
# 0 "" 2
# 60 "lwp.c" 1
	popq  %r13
# 0 "" 2
# 60 "lwp.c" 1
	popq  %r12
# 0 "" 2
# 60 "lwp.c" 1
	popq  %r11
# 0 "" 2
# 60 "lwp.c" 1
	popq  %r10
# 0 "" 2
# 60 "lwp.c" 1
	popq  %r9
# 0 "" 2
# 60 "lwp.c" 1
	popq  %r8
# 0 "" 2
# 60 "lwp.c" 1
	popq  %rdi
# 0 "" 2
# 60 "lwp.c" 1
	popq  %rsi
# 0 "" 2
# 60 "lwp.c" 1
	popq  %rdx
# 0 "" 2
# 60 "lwp.c" 1
	popq  %rcx
# 0 "" 2
# 60 "lwp.c" 1
	popq  %rbx
# 0 "" 2
# 60 "lwp.c" 1
	popq  %rax
# 0 "" 2
# 60 "lwp.c" 1
	movq  %rbp,%rsp
# 0 "" 2
#NO_APP
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	lwp_yield, .-lwp_yield
	.globl	lwp_exit
	.type	lwp_exit, @function
lwp_exit:
.LFB5:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	lwp_running(%rip), %eax
	cltq
	salq	$5, %rax
	addq	$lwp_ptable, %rax
	movq	8(%rax), %rax
	movq	%rax, %rdi
	call	free
	movl	lwp_running(%rip), %eax
	addl	$1, %eax
	movl	%eax, -4(%rbp)
	jmp	.L12
.L13:
	movl	-4(%rbp), %eax
	subl	$1, %eax
	cltq
	salq	$5, %rax
	addq	$lwp_ptable, %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	salq	$5, %rdx
	addq	$lwp_ptable, %rdx
	movq	(%rdx), %rcx
	movq	%rcx, (%rax)
	movq	8(%rdx), %rcx
	movq	%rcx, 8(%rax)
	movq	16(%rdx), %rcx
	movq	%rcx, 16(%rax)
	movq	24(%rdx), %rdx
	movq	%rdx, 24(%rax)
	addl	$1, -4(%rbp)
.L12:
	movl	lwp_procs(%rip), %eax
	cmpl	%eax, -4(%rbp)
	jl	.L13
	movl	lwp_procs(%rip), %eax
	subl	$1, %eax
	movl	%eax, lwp_procs(%rip)
	movl	lwp_procs(%rip), %eax
	testl	%eax, %eax
	jne	.L14
	movl	$0, %eax
	call	lwp_stop
	jmp	.L11
.L14:
	movq	global_scheduler(%rip), %rax
	testq	%rax, %rax
	jne	.L16
	movl	$0, lwp_running(%rip)
	jmp	.L17
.L16:
	movq	global_scheduler(%rip), %rax
	call	*%rax
	movl	%eax, lwp_running(%rip)
.L17:
	movl	lwp_running(%rip), %eax
	cltq
	salq	$5, %rax
	addq	$lwp_ptable+16, %rax
	movq	8(%rax), %rax
#APP
# 79 "lwp.c" 1
	movq  %rax,%rsp
# 0 "" 2
# 80 "lwp.c" 1
	popq  %rbp
# 0 "" 2
# 80 "lwp.c" 1
	popq  %r15
# 0 "" 2
# 80 "lwp.c" 1
	popq  %r14
# 0 "" 2
# 80 "lwp.c" 1
	popq  %r13
# 0 "" 2
# 80 "lwp.c" 1
	popq  %r12
# 0 "" 2
# 80 "lwp.c" 1
	popq  %r11
# 0 "" 2
# 80 "lwp.c" 1
	popq  %r10
# 0 "" 2
# 80 "lwp.c" 1
	popq  %r9
# 0 "" 2
# 80 "lwp.c" 1
	popq  %r8
# 0 "" 2
# 80 "lwp.c" 1
	popq  %rdi
# 0 "" 2
# 80 "lwp.c" 1
	popq  %rsi
# 0 "" 2
# 80 "lwp.c" 1
	popq  %rdx
# 0 "" 2
# 80 "lwp.c" 1
	popq  %rcx
# 0 "" 2
# 80 "lwp.c" 1
	popq  %rbx
# 0 "" 2
# 80 "lwp.c" 1
	popq  %rax
# 0 "" 2
# 80 "lwp.c" 1
	movq  %rbp,%rsp
# 0 "" 2
#NO_APP
.L11:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	lwp_exit, .-lwp_exit
	.globl	lwp_start
	.type	lwp_start, @function
lwp_start:
.LFB6:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	lwp_procs(%rip), %eax
	testl	%eax, %eax
	jne	.L19
	jmp	.L18
.L19:
#APP
# 88 "lwp.c" 1
	pushq %rax
# 0 "" 2
# 88 "lwp.c" 1
	pushq %rbx
# 0 "" 2
# 88 "lwp.c" 1
	pushq %rcx
# 0 "" 2
# 88 "lwp.c" 1
	pushq %rdx
# 0 "" 2
# 88 "lwp.c" 1
	pushq %rsi
# 0 "" 2
# 88 "lwp.c" 1
	pushq %rdi
# 0 "" 2
# 88 "lwp.c" 1
	pushq %r8
# 0 "" 2
# 88 "lwp.c" 1
	pushq %r9
# 0 "" 2
# 88 "lwp.c" 1
	pushq %r10
# 0 "" 2
# 88 "lwp.c" 1
	pushq %r11
# 0 "" 2
# 88 "lwp.c" 1
	pushq %r12
# 0 "" 2
# 88 "lwp.c" 1
	pushq %r13
# 0 "" 2
# 88 "lwp.c" 1
	pushq %r14
# 0 "" 2
# 88 "lwp.c" 1
	pushq %r15
# 0 "" 2
# 88 "lwp.c" 1
	pushq %rbp
# 0 "" 2
# 89 "lwp.c" 1
	movq  %rsp,%rax
# 0 "" 2
#NO_APP
	movq	%rax, global_stack_pointer(%rip)
	movq	global_scheduler(%rip), %rax
	testq	%rax, %rax
	jne	.L21
	movl	$0, lwp_running(%rip)
	jmp	.L22
.L21:
	movq	global_scheduler(%rip), %rax
	call	*%rax
	movl	%eax, lwp_running(%rip)
.L22:
	movl	lwp_running(%rip), %eax
	cltq
	salq	$5, %rax
	addq	$lwp_ptable+16, %rax
	movq	8(%rax), %rax
#APP
# 97 "lwp.c" 1
	movq  %rax,%rsp
# 0 "" 2
# 98 "lwp.c" 1
	popq  %rbp
# 0 "" 2
# 98 "lwp.c" 1
	popq  %r15
# 0 "" 2
# 98 "lwp.c" 1
	popq  %r14
# 0 "" 2
# 98 "lwp.c" 1
	popq  %r13
# 0 "" 2
# 98 "lwp.c" 1
	popq  %r12
# 0 "" 2
# 98 "lwp.c" 1
	popq  %r11
# 0 "" 2
# 98 "lwp.c" 1
	popq  %r10
# 0 "" 2
# 98 "lwp.c" 1
	popq  %r9
# 0 "" 2
# 98 "lwp.c" 1
	popq  %r8
# 0 "" 2
# 98 "lwp.c" 1
	popq  %rdi
# 0 "" 2
# 98 "lwp.c" 1
	popq  %rsi
# 0 "" 2
# 98 "lwp.c" 1
	popq  %rdx
# 0 "" 2
# 98 "lwp.c" 1
	popq  %rcx
# 0 "" 2
# 98 "lwp.c" 1
	popq  %rbx
# 0 "" 2
# 98 "lwp.c" 1
	popq  %rax
# 0 "" 2
# 98 "lwp.c" 1
	movq  %rbp,%rsp
# 0 "" 2
#NO_APP
.L18:
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	lwp_start, .-lwp_start
	.globl	lwp_stop
	.type	lwp_stop, @function
lwp_stop:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
#APP
# 102 "lwp.c" 1
	pushq %rax
# 0 "" 2
# 102 "lwp.c" 1
	pushq %rbx
# 0 "" 2
# 102 "lwp.c" 1
	pushq %rcx
# 0 "" 2
# 102 "lwp.c" 1
	pushq %rdx
# 0 "" 2
# 102 "lwp.c" 1
	pushq %rsi
# 0 "" 2
# 102 "lwp.c" 1
	pushq %rdi
# 0 "" 2
# 102 "lwp.c" 1
	pushq %r8
# 0 "" 2
# 102 "lwp.c" 1
	pushq %r9
# 0 "" 2
# 102 "lwp.c" 1
	pushq %r10
# 0 "" 2
# 102 "lwp.c" 1
	pushq %r11
# 0 "" 2
# 102 "lwp.c" 1
	pushq %r12
# 0 "" 2
# 102 "lwp.c" 1
	pushq %r13
# 0 "" 2
# 102 "lwp.c" 1
	pushq %r14
# 0 "" 2
# 102 "lwp.c" 1
	pushq %r15
# 0 "" 2
# 102 "lwp.c" 1
	pushq %rbp
# 0 "" 2
#NO_APP
	movq	global_stack_pointer(%rip), %rax
#APP
# 103 "lwp.c" 1
	movq  %rax,%rsp
# 0 "" 2
# 104 "lwp.c" 1
	popq  %rbp
# 0 "" 2
# 104 "lwp.c" 1
	popq  %r15
# 0 "" 2
# 104 "lwp.c" 1
	popq  %r14
# 0 "" 2
# 104 "lwp.c" 1
	popq  %r13
# 0 "" 2
# 104 "lwp.c" 1
	popq  %r12
# 0 "" 2
# 104 "lwp.c" 1
	popq  %r11
# 0 "" 2
# 104 "lwp.c" 1
	popq  %r10
# 0 "" 2
# 104 "lwp.c" 1
	popq  %r9
# 0 "" 2
# 104 "lwp.c" 1
	popq  %r8
# 0 "" 2
# 104 "lwp.c" 1
	popq  %rdi
# 0 "" 2
# 104 "lwp.c" 1
	popq  %rsi
# 0 "" 2
# 104 "lwp.c" 1
	popq  %rdx
# 0 "" 2
# 104 "lwp.c" 1
	popq  %rcx
# 0 "" 2
# 104 "lwp.c" 1
	popq  %rbx
# 0 "" 2
# 104 "lwp.c" 1
	popq  %rax
# 0 "" 2
# 104 "lwp.c" 1
	movq  %rbp,%rsp
# 0 "" 2
#NO_APP
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	lwp_stop, .-lwp_stop
	.globl	lwp_set_scheduler
	.type	lwp_set_scheduler, @function
lwp_set_scheduler:
.LFB8:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, global_scheduler(%rip)
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	lwp_set_scheduler, .-lwp_set_scheduler
	.ident	"GCC: (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4"
	.section	.note.GNU-stack,"",@progbits
