[bits 64]
global _start
;global __stack_chk_guard
;global __stack_chk_fail

extern _libc_setup

section .text
_start:
;    call __stack_chk_init
    call _libc_setup ; we dont call main anymore because _libc_setup will
                     ; do whatever setup needed and call main for us
                     ; also we dont needa do setup since ld.so just invokes
                     ; this like a C function
    ret

;__stack_chk_init:
;    push rax
;    push rdx
;
;    rdtsc
;    shl rdx, 32
;    or rax, rdx
;    mov [__stack_chk_guard], rax
;
;    pop rdx
;    pop rax
;
;    mov rax, 3
;    mov rdi, 2
;    lea rsi, [__tst]
;    mov rdx, 7
;    syscall
;
;    ret

;__stack_chk_fail:
;    mov rax, 3
;    mov rdi, 2
;    lea rsi, [__stack_chk_fail_msg]
;    mov rdx, 32
;    syscall
;
;    mov rax, 1
;    mov rdi, 1
;    syscall

;section .data
;__stack_chk_guard: dq 0
;__stack_chk_fail_msg: db "*** STACK SMASHING DETECTED ***",0xa
;__tst: db "check",0xa