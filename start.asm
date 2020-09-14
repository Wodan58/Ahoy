;
;   module  : start.asm
;   version : 1.1
;   date    : 09/12/20
;
	bits 64	
	default rel
extern main
global _start, _exit
global my_read, my_write, my_open, my_close, my_break, my_clock
	section .text
_start:
	mov rdi, [rsp]		; argc
	lea rsi, [rsp+8]	; argv
	call main
	mov rdi, rax		; return
_exit:
	mov rax, 60		; exit
	syscall
my_read:
	xor rax, rax		; read
	syscall
	ret
my_write:
	mov rax, 1		; write
	syscall
	ret
my_open:
	mov rax, 2		; open
	syscall
	ret
my_close:
	mov rax, 3		; close
	syscall
	ret
my_break:
	mov rax, 12		; brk
	syscall
	ret
my_clock:
	mov rdi, clk
	xor rsi, rsi		; timezone
	mov rax, 96		; gettimeofday
	syscall
	mov rax, [clk]
	imul rax, 1000000
	add rax, [clk+8]
	ret
	section .bss
clk:
	resq	2
