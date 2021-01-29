extern isrDefaultInt, isrGPExc, isrPFexc, isrClockInt, isrKbdInt, doSyscalls
global asmDefaultInt, asmExcGP, asmExcPF, asmIrq0, asmIrq1, asmSyscalls 

%macro  SAVE_REGS 0
        pushad 
        push ds
        push es
        push fs
        push gs 
        push ebx
        mov bx,0x10
        mov ds,bx
        pop ebx
%endmacro

%macro  RESTORE_REGS 0
        pop gs
        pop fs
        pop es
        pop ds
        popad
%endmacro

asmDefaultInt:
        SAVE_REGS
        call isrDefaultInt
        mov al,0x20
        out 0x20,al
        RESTORE_REGS
        iret

asmExcGP:
        SAVE_REGS
        call isrGPExc
        RESTORE_REGS
        add esp,4
        iret

asmExcPF:
        SAVE_REGS
        call isrPFexc
        RESTORE_REGS
        add esp,4
        iret

asmIrq0:
        SAVE_REGS
        call isrClockInt
        mov al,0x20
        out 0x20,al
        RESTORE_REGS        
        iret

asmIrq1:
        SAVE_REGS
        call isrKbdInt
        mov al,0x20
        out 0x20,al
        RESTORE_REGS
        iret

asmSyscalls:
        SAVE_REGS
        push eax             
        call doSyscalls
        pop eax
        RESTORE_REGS
        iret


