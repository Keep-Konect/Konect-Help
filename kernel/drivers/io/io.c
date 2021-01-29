#define __io__
#include "io.h"
#include "../../regs/reg.h"
#include "../../screen/screen.h"

void cli(){
	asm("cli"::);
}

void sti(){
	asm("sti"::);
}

int inb(int port) {
    unsigned char _v;
    asm volatile ("inb %%dx, %%al" : "=a" (_v) : "d" (port));
    return _v;
}

void outbp(int port, int value){
    asm volatile ("outb %%al, %%dx; jmp 1f; 1:" :: "d" (port), "a" (value));			
}

void outb(int port, int value){
	asm volatile ("outb %%al, %%dx" :: "d" (port), "a" (value));
}

void callBIOS(int interruption, RegisterAsm* in, RegisterAsm* out){
    go16();
    printk("Ax value");
}
