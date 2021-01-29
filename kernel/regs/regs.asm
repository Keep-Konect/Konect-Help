global getAX, setAX

setAX:
    push ebp
    mov ebp, esp
    mov ax, [ebp+8] 
    mov esp, ebp
    pop ebp
    ret

getAX:
    ret
    