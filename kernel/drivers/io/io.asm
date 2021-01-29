global go16
;______________________________________________________________________________________________________
;Switch to 16-bit real Mode
;IN/OUT:  nothing

go16:
    [BITS 32]
    ;cli                 ;Clear interrupts    
    pop edx             ;save return location in edx
    jmp 0x20:PM16       ;Load CS with selector 0x20
;For go to 16-bit real mode, first we have to go to 16-bit protected mode
    [BITS 16]
PM16:
    mov ax, 0x28        ;0x28 is 16-bit protected mode selector.
    mov ss, ax  
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    mov sp, 0x7c00+0x200    ;Stack hase base at 0x7c00+0x200    

	
    mov eax, cr0
    and eax, 0xfffffffe ;Clear protected enable bit in cr0
    mov cr0, eax  
    hlt 
    jmp 0x50:realMode   ;Load CS and IP

realMode:
;Load segment registers with 16-bit Values.
    mov ax, 0x50
    mov ds, ax
    mov fs, ax
    mov gs, ax
    mov ax, 0
    mov ss, ax
    mov ax, 0
    mov es, ax
    mov sp, 0x7c00+0x200    
    lidt[.idtR]     ;Load real mode interrupt vector table
    sti

    push 0x50       ;New CS
    push dx         ;New IP (saved in edx)
    retf            ;Load CS, IP and Start real mode

;Real mode interrupt vector table
.idtR: 
	dw 0x03FF       ;Limit
    dd 0            ;Base

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