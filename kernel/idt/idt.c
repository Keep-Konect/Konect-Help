#include "../screen/screen.h"
#include "../types/types.h"
#include "../drivers/io/io.h"
#include "../pic/pic.h"
#include "../lib/lib.h"

#define __IDT__

#include "idt.h"

extern void asmDefaultInt(void);
extern void isrGPExc(void);
extern void asmExcPF(void);
extern void asmIrq0(void);
extern void asmIrq1(void);
extern void asmSyscalls(void);

void init_idt_desc(u16 select, u32 offset, u16 type, struct idtdesc *desc)
{
	desc->offset0_15 = (offset & 0xffff);
	desc->select = select;
	desc->type = type;
	desc->offset16_31 = (offset & 0xffff0000) >> 16;
	return;
}

void init_idt(void)
{
	int i;
	for (i = 0; i < IDTSIZE; i++){
		init_idt_desc(0x08, (u32) asmDefaultInt, INTGATE, &kidt[i]);
	}

	init_idt_desc(0x08, (u32) isrGPExc, INTGATE, &kidt[13]); //gp
	init_idt_desc(0x08, (u32) asmExcPF, INTGATE, &kidt[14]);  //pf

	init_idt_desc(0x08, (u32) asmIrq0, INTGATE, &kidt[32]); //timer
	init_idt_desc(0x08, (u32) asmIrq1, INTGATE, &kidt[33]);	//keyboard

	init_idt_desc(0x08, (u32) asmSyscalls, TRAPGATE, &kidt[48]); //system call

	kidtr.limite = IDTSIZE * 8;
	kidtr.base = IDTBASE;

	memcpy((char *) kidtr.base, (char *) kidt, kidtr.limite);

	asm("lidtl (kidtr)");
}
