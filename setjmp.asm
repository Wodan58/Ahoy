;
;   module  : setjmp.asm
;   version : 1.1
;   date    : 09/12/20
;
	bits 64	
global my_setjmp, my_longjmp
	section .text
my_setjmp:
	mov rax, [rsp]
	mov [rdi], rsp		; stackp
	mov [rdi+8], rax	; return
	mov [rdi+16], rbx
	mov [rdi+24], rbp
	mov [rdi+32], r12
	mov [rdi+40], r13
	mov [rdi+48], r14
	mov [rdi+56], r15
	xor rax, rax		; rvalue
	ret
my_longjmp:
	mov rsp, [rdi]		; stackp
	mov rax, [rdi+8]	; return
	mov [rsp], rax
	mov rbx, [rdi+16]
	mov rbp, [rdi+24]
	mov r12, [rdi+32]
	mov r13, [rdi+40]
	mov r14, [rdi+48]
	mov r15, [rdi+56]
	mov rax, rsi		; rvalue
	ret
