#include "../../../types/types.h"
#include "../../../drivers/io/io.h"
#include "../../../lib/lib.h"
#include "../../../mm/mm.h"
#include "../../../kmalloc/kmalloc.h"

#define __PLIST__
#include "process.h"


int load_task(char *fn, u32 code_size)
{
	struct page_directory *pd;
	struct page_list *pglist;
	struct page *kstack;

	char *v_addr;
	char *p_addr;
	char *ustack;

	int pid;
	int i;

	pid = 1;
	while (p_list[pid].state != 0 && pid++ < MAXPID);

	if (p_list[pid].state != 0) {
		printk("PANIC: not enough slot for processes\n");
		return 0;
	}

	pd = pd_create();

	asm("mov %0, %%eax; mov %%eax, %%cr3"::"m"(pd->base->p_addr));

	pglist = (struct page_list *) kmalloc(sizeof(struct page_list));
	pglist->page = 0;
	pglist->next = 0;
	pglist->prev = 0;

	i = 0;
	while (i < code_size) {
		p_addr = get_page_frame();
		v_addr = (char *) (USER_OFFSET + i);
		pd_add_page(v_addr, p_addr, PG_USER, pd);

		pglist->page = (struct page *) kmalloc(sizeof(struct page));
		pglist->page->p_addr = p_addr;
		pglist->page->v_addr = v_addr;

		pglist->next = (struct page_list *) kmalloc(sizeof(struct page_list));
		pglist->next->page = 0;
		pglist->next->next = 0;
		pglist->next->prev = pglist;

		pglist = pglist->next;

		i += PAGESIZE;
	};

	memcpy((char *) USER_OFFSET, fn, code_size);

	ustack = get_page_frame();
	pd_add_page((char *) USER_STACK, ustack, PG_USER, pd);

	kstack = get_page_from_heap();

	n_proc++;

	p_list[pid].pid = pid;

	p_list[pid].regs.ss = 0x33;
	p_list[pid].regs.esp = USER_STACK + PAGESIZE - 16;
	p_list[pid].regs.eflags = 0x0;
	p_list[pid].regs.cs = 0x23;
	p_list[pid].regs.eip = 0x40000000;
	p_list[pid].regs.ds = 0x2B;
	p_list[pid].regs.es = 0x2B;
	p_list[pid].regs.fs = 0x2B;
	p_list[pid].regs.gs = 0x2B;

	p_list[pid].regs.cr3 = (u32) pd->base->p_addr;

	p_list[pid].kstack.ss0 = 0x18;
	p_list[pid].kstack.esp0 = (u32) kstack->v_addr + PAGESIZE - 16;

	p_list[pid].regs.eax = 0;
	p_list[pid].regs.ecx = 0;
	p_list[pid].regs.edx = 0;
	p_list[pid].regs.ebx = 0;

	p_list[pid].regs.ebp = 0;
	p_list[pid].regs.esi = 0;
	p_list[pid].regs.edi = 0;

	p_list[pid].pd = pd;
	p_list[pid].pglist = pglist;

	p_list[pid].state = 1;

	asm("mov %0, %%eax ;mov %%eax, %%cr3":: "m"(current->regs.cr3));

	return pid;
}