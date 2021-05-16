bits 64

global main

section .text 

%define     readmode    0
%define     writemode   4
%define     stdin       0
%define     stdout      2
%define     bufferSize  64
%define     system      80h
%define     exitcode    3ch

jmp                 main

_callHLT: 
    jmp _HLT

_callIN:
    jmp _IN

_callOUT:
    jmp _OUT

_HLT:
    mov	            eax, 1		            ;exit code
    mov             ebx, 0
    int	            80h	
    ret

_IN:
    mov             r10, rsp                ;remember stack pointer

    sub             rsp, bufferSize         ;size of buffer
    mov             rsi, rsp

    
    mov             rax, readmode           ;
    mov             rdi, stdin              ;
    mov             rsi, rsp                ;INPUT        
    mov             rdx, bufferSize         ;
    syscall                                 ;

    dec rcx

    call            _handle_numeric_string

    mov             rsp, r10                ;returning stack pointer

    pop             r10
    sub             rsp, 8
    movsd           [rsp], xmm0       ;push this shit into stack
    push            r10

    ret


_OUT:
    pop             r10
    movsd           xmm0, [rsp]
    add             rsp, 8
    push            r10

    mov             rax, 100000
    cvtsi2sd        xmm1, rax
    mulsd           xmm0, xmm1
    cvtsd2si        eax, xmm0

    mov             r10, rsp
    sub             rsp, bufferSize
    mov             rsi, rsp
    mov             rdi, rsi

    mov             rbx, 1
    mov             rcx, 0

    cmp              eax, 0xF000000
    jb               NoMinus

    mov             byte [rsi], '-'     ;
    inc             rsi                 ;if minus
    neg             eax

    NoMinus:
    cmp             eax, 100000
    jae             max10pow

    mov             byte [rsi], '0'     ;
    inc             rsi                 ;if abs is less than 1

    jmp             max10pow

    
    next:
    cmp rcx, 5
    jae big

    mov rcx,5

    big: 
    
    add             rsi, rcx

    mov             r13, rsi
    sub             r13, rdi        ;length of number
    inc             r13

    mov             r12, 10

    xor             rdx,rdx
    div             r12
    add             rdx, '0'
    mov             byte [rsi], dl
    dec             rsi

    xor             rdx,rdx
    div             r12
    add             rdx, '0'
    mov             byte [rsi], dl
    dec             rsi

    xor             rdx,rdx
    div             r12
    add             rdx, '0'
    mov             byte [rsi], dl
    dec             rsi

    xor             rdx,rdx
    div             r12
    add             rdx, '0'
    mov             byte [rsi], dl
    dec             rsi

    xor             rdx,rdx
    div             r12
    add             rdx, '0'
    mov             byte [rsi], dl
    dec             rsi

    sub             rcx, 4

    mov             byte [rsi], '.'     ;
    dec             rsi                 ;dot add

    output_cycle_BD:
    cmp             rcx, 1
    jbe             output_call
    
    xor             rdx,rdx    
    div             r12
    fix:
    add             dl, '0'
    mov             byte [rsi], dl
    dec             rsi
    dec             rcx
    jmp             output_cycle_BD
    
    output_call:

    mov             rax, 1                  ; write
    mov             rsi, rdi
    mov             rdi, 1                  ; stdout
    mov             rdx, r13                ; length
    syscall



    add             rsp, bufferSize
    ret

    max10pow:

    cmp             rax, rbx
    jb              next

    imul            rbx,rbx,10
    inc             rcx
    jmp             max10pow
                        


;---------------------------------------------
;rsi - begin pf buffer, rax - length of string
;
;
;
;----------------------------------------------


_handle_numeric_string:
    mov         rcx, rax                ;

    xor         rax, rax                ;
    xor         rbx, rbx                ;
    xor         rdx, rdx                ;counting symbols after dot
    mov         r11, 1

    dec         rcx

    mov         bh, [rsi]
    cmp         bh, '-' 
    jne         numeric_handle

    mov         r11, -1                 ;setting minus
    inc         rsi
    dec         rcx

    numeric_handle:
        xor         rbx,rbx
        mov         bl, [rsi]
        inc         rsi
        dec         rcx
        cmp         bl, '.'
        je          dot_numeric_handle

        sub         bl, '0'
        imul        rax, rax, 10
        add         rax, rbx
        
        cmp         rcx, 0 
        je          float_transform

    jmp numeric_handle


    dot_numeric_handle:
        cmp         rcx, 0 
        je          float_transform

        xor         rbx,rbx
        mov         bl, [rsi]
        inc         rsi
        dec         rcx

        sub         bl, '0'
        imul        rax, rax, 10
        add         rax, rbx

        inc         rdx                ;counting numbers after dot

    jmp dot_numeric_handle
        

    float_transform:
        imul            rax, r11
        cvtsi2sd        xmm0,rax

        mov             r11, 1
        precision_cycle:
            cmp         rdx, 0
            je          set_presicion

            imul        r11, r11, 10
            dec         rdx
            jmp precision_cycle

    set_presicion:
        cvtsi2sd        xmm1, r11
        divsd           xmm0, xmm1

    ret

main: 
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
call _callIN
call _callIN
movsd xmm0, [rsp]
add   rsp, 8
movsd xmm1, [rsp]
add   rsp, 8
cmpsd xmm0, xmm1, 0
ptest xmm0, xmm0
;movsd  rax, xmm0
;cmp rax, 0
je _callHLT
in:
call _callIN
call _callOUT
call _callHLT
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop









