#include "../types/types.h"
#include "../gdt/gdt.h"
#include "../screen/screen.h"

void doSyscalls(int sys_num)
{
	char *u_str;
	int i;

	if(sys_num == 1){
		asm("mov %%ebx, %0": "=m"(u_str) :);
		for (i = 0; i < 100000; i++);  
		cli();
		printf(u_str);
		sti();
	}else{
		printf("unknown syscall \n");
	}

	return;
}