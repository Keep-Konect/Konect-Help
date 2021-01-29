#include "interrupt.h"

void isrDefaultInt(void)
{
        
}

void isrGPExc(void){
	print("GP fault\n", 0x04);
	asm("hlt");
}

void isrPFexc(void){
        u32 faulting_addr;
        u32 eip;

        asm("   movl 60(%%ebp), %%eax; \
                mov %%eax, %0;         \
                mov %%cr2, %%eax;      \
                mov %%eax, %1": "=m"(eip), "=m"(faulting_addr): );

        printk("PF fault on eip: %p. cr2: %p\n", eip, faulting_addr);
        dump((uchar *) &faulting_addr, 4);
        dump((uchar *) &eip, 4);

        asm("hlt");
}

void isrClockInt(void)
{
        static int tic = 0;
        static int sec = 0;
        tic++;
        if (tic % 100 == 0) {
                sec++;                
                tic = 0;
        }
        schedule();
}

void isrKbdInt(void)
{
        onKeyClick();
}