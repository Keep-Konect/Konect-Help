#include "../handlers/sound/sound.h"
#include "../screen/screen.h"
#include "../gdt/gdt.h"
#include "../types/types.h"
#include "../drivers/io/io.h"
#include "../drivers/keyboard/keyboard.h"
#include "../pic/pic.h"
#include "../idt/idt.h"
#include "../mm/mm.h"
#include "../handlers/task/process/process.h"

int main(void);
void task1(void);
void task2(void);
void task3(void);
extern void turnToOff();

void _start(void)
{	
	print("Kernel is load successfully\n", 0x0E);	
	//init gdt
	init_gdt();
	print("kernel : new gdt loaded !\n", 0x0E);
	go16();
	//Initialization of the stack pointer %esp
	asm("   movw $0x18, %ax \n \
        	movw %ax, %ss \n \
            movl $0x20000, %esp");	

	//init idt
	init_idt();
	print("kernel : idt loaded\n", 0x0E);

	//init pic
    init_pic();
    print("kernel : pic configured\n", 0x0E);

	//init tss
	asm("	movw $0x38, %ax \n \
			ltr %ax");
	print("kernel : tr loaded\n", 0x0E);
	//init paging
	//init_mm();
	print("kernel : paging enable\n", 0x0E);  

	print("kernel : scheduler enabled\n", 0x0E);	
	main();
}

int main(void)
{
	kkybrd_enable();
	kkybrd_set_leds(1, 1, 1);
	sti();
	while(1);
}