;---------------------------------------------------------
%define BASE 0x100
%define KSIZE 50
;---------------------------------------------------------

;---------------------------------------------------------
[BITS 16]
[ORG 0x0]
;---------------------------------------------------------

;---------------------------------------------------------
jmp start

;---------------------------------------------------------
; macro
%macro descInit    5    ; base(32), limite(20/32), acces(8), flags(4/8), adresse(32)
    push eax
; base :
    mov eax, %1
    mov word [%5+2], ax
    shr eax, 16
    mov byte [%5+4], al
    shr eax, 8
    mov byte [%5+7], al
; limite :
    mov eax, %2
    and eax, 0x000FFFFF
    mov word [%5], ax 
    shr eax, 16       
    mov byte [%5+6], 0 
    or  [%5+6], al     
; flags :
    mov al, %4
    and al, 0x0F
    shl al, 4
    or [%5+6], al
; acces :
    mov byte [%5+5], %3
    pop eax
%endmacro

;---------------------------------------------------------

start:
; initialization of the segments in 0x07C00

; define ax as 0x07c0
    mov ax, 0x07c0
; define ds as ax
    mov ds, ax
; define es as ax
    mov es, ax

; define ax as 0x8000
    mov ax, 0x8000
; define ds ss ax
    mov ss, ax

; define sp as 0xf000
    mov sp, 0xf000

;---------------------------------------------------------
; recovery of the boot unit
    mov [bootdrv], dl
;---------------------------------------------------------
; switch graphic mode
;    call switchGraphicMode
;---------------------------------------------------------
; show the message
    mov si, StartMsg
    call print
;---------------------------------------------------------
; load the kernel
    xor ax, ax
    int 0x13

    push es
    mov ax, BASE
    mov es, ax
    mov bx, 0
    mov ah, 2
    mov al, KSIZE
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [bootdrv]
    int 0x13
    pop es

;---------------------------------------------------------
; init gdt 
    descInit 0, 0xFFFFF, 10011011b, 1101b, gdt_cs
    descInit 0, 0xFFFFF, 10010011b, 1101b, gdt_ds

;---------------------------------------------------------
; initialization of the pointer to the GDT
    mov ax, gdtend
    mov bx, gdt
    sub ax, bx
    mov word [gdtptr], ax

    xor eax, eax
    mov ax, ds
    mov bx, gdt
    call calcadr
    mov dword [gdtptr+2], ecx

;---------------------------------------------------------
; switch to protected mode
    cli
    lgdt [gdtptr]
    mov eax, cr0
    or  ax, 1
    mov cr0, eax

;---------------------------------------------------------
    jmp next

;---------------------------------------------------------
next:
    mov ax, 0x10
    mov ds, ax
    mov fs, ax
    mov gs, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x9F000


;---------------------------------------------------------
; jump to the kernel
    jmp dword 0x8:0x1000

;----------------------the variables----------------------
bootdrv:  db 0
StartMsg: db "Kernel is loading", 13, 10, 0
;---------------------------------------------------------
gdt:
gdt_null:
    dw 0, 0, 0, 0
gdt_cs:
    dw 0, 0, 0, 0
gdt_ds:
    dw 0, 0, 0, 0
gdtend:
gdtptr:
    dw 0x0000
    dd 0
;---------------------------------------------------------

;----------------------the functions----------------------
switchGraphicMode:
    mov ah, 0
    mov ax, 0x4F02 ; here select which mode you want
    mov bx, 0x011B
    int 16

print:
    push ax
    push bx

.startMsg:
    lodsb
    cmp al, 0
    jz .endMsg
    mov ah, 0x0E
    mov bx, 0x07
    int 0x10
    jmp .startMsg

.endMsg:
    pop bx
    pop ax
    ret

calcadr:
    xor ecx, ecx
    mov cx, ax
    shl ecx, 4
    and ebx, 0x0000FFFF
    add ecx, ebx
    ret

;---------------------------the timer---------------------
; NOP timer
    times 510-($-$$) db 0
    dw 0xAA55
;---------------------------------------------------------
