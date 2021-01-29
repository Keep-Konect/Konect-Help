#include "../types/types.h"
#include "../lib/lib.h"
#include "../drivers/io/io.h"
#include "../handlers/task/process/process.h"

#define __MM__
#include "mm.h"

#include "../kmalloc/kmalloc.h"

void set_page_frame_used(int page){
    mem_bitmap[((u32) page)/8] |= (1 << (((u32) page)%8));
}  

void release_page_frame(int p_addr){
    mem_bitmap[((u32) p_addr/PAGESIZE)/8] &= ~(1 << (((u32) p_addr/PAGESIZE)%8));
}      

char* get_page_frame(void)
{
	int byte, bit;
	int page = -1;

	for (byte = 0; byte < RAM_MAXPAGE / 8; byte++)
		if (mem_bitmap[byte] != 0xFF)
			for (bit = 0; bit < 8; bit++)
				if (!(mem_bitmap[byte] & (1 << bit))) {
					page = 8 * byte + bit;
					set_page_frame_used(page);
					return (char *) (page * PAGESIZE);
				}
	return (char *) -1;
}


struct page* get_page_from_heap(void)
{
	struct page *pg;
	struct vm_area *p;
	char *v_addr, *p_addr;

	p_addr = get_page_frame();
	if (p_addr < 0) {
		printk ("PANIC: get_page_from_heap(): no page frame available. System halted !\n");
		asm("hlt");
	}

	if (free_vm->vm_end == free_vm->vm_start) {
		printk ("PANIC: get_page_from_heap(): not memory left in page heap. System halted !\n");
		asm("hlt");
	}

	v_addr = free_vm->vm_start;

	if (free_vm->vm_end - free_vm->vm_start == PAGESIZE) {
		if (free_vm->next) {
			p = free_vm;
			free_vm = free_vm->next;
			free_vm->prev = 0;
			kfree(p);
		}
	}
	else {
		free_vm->vm_start += PAGESIZE;
	}

	pd0_add_page(v_addr, p_addr, 0);

	pg = (struct page*) kmalloc(sizeof(struct page));
	pg->v_addr = v_addr;
	pg->p_addr = p_addr;

	return pg;
}

int release_page_from_heap(char *v_addr)
{
	struct vm_area *p, *to_del, *new_vm_area;
	char *p_addr;

	p_addr = get_p_addr(v_addr);
	if (p_addr) {
		release_page_frame(p_addr);
	}
	else {
		printk("WARNING: release_page_from_heap(): no page frame associated with v_addr %x\n", v_addr);
		return 1;
	}

	pd_remove_page(v_addr);

	p = free_vm;
	while (p->vm_start < v_addr && p->next)
		p = p->next;

	if (v_addr + PAGESIZE == p->vm_start) {
		p->vm_start = v_addr;
		if (p->prev && p->prev->vm_end == p->vm_start) {
			to_del = p->prev;
			p->vm_start = p->prev->vm_start;
			p->prev = p->prev->prev;
			if (p->prev)
				p->prev->next = p;
			kfree(to_del);
		}
	}
	else if (p->prev && p->prev->vm_end == v_addr) {
		p->prev->vm_end += PAGESIZE;
	}
	else if (v_addr + PAGESIZE < p->vm_start) {
		new_vm_area = (struct vm_area*) kmalloc(sizeof(struct vm_area));
		new_vm_area->vm_start = v_addr;
		new_vm_area->vm_end = v_addr + PAGESIZE;
		new_vm_area->prev = p->prev;
		new_vm_area->next = p;
		p->prev = new_vm_area;
		if (new_vm_area->prev)
			new_vm_area->prev->next = new_vm_area;
	}
	else {
		printk ("PANIC: release_page_from_heap(): corrupted linked list. System halted !\n");
		asm("hlt");
	}

	return 0;
}

void init_mm(u32 high_mem)
{
	int pg, pg_limit;
	unsigned long i;

	pg_limit = (high_mem * 1024) / PAGESIZE;

	for (pg = 0; pg < pg_limit / 8; pg++)
		mem_bitmap[pg] = 0;

	for (pg = pg_limit / 8; pg < RAM_MAXPAGE / 8; pg++)
		mem_bitmap[pg] = 0xFF;

	for (pg = PAGE(0x0); pg < PAGE((u32) pg1_end); pg++) {
		set_page_frame_used(pg);
	}

	pd0[0] = ((u32) pg0 | (PG_PRESENT | PG_WRITE | PG_4MB));
	pd0[1] = ((u32) pg1 | (PG_PRESENT | PG_WRITE | PG_4MB));
	for (i = 2; i < 1023; i++)
		pd0[i] =
		    ((u32) pg1 + PAGESIZE * i) | (PG_PRESENT | PG_WRITE);

	pd0[1023] = ((u32) pd0 | (PG_PRESENT | PG_WRITE));

	asm("	mov %0, %%eax \n \
		mov %%eax, %%cr3 \n \
		mov %%cr4, %%eax \n \
		or %2, %%eax \n \
		mov %%eax, %%cr4 \n \
		mov %%cr0, %%eax \n \
		or %1, %%eax \n \
		mov %%eax, %%cr0"::"m"(pd0), "i"(PAGING_FLAG), "i"(PSE_FLAG));

	kern_heap = (char *) KERN_HEAP;
	ksbrk(1);

	free_vm = (struct vm_area*) kmalloc(sizeof(struct vm_area));
	free_vm->vm_start = (char*) KERN_PG_HEAP;
	free_vm->vm_end = (char*) KERN_PG_HEAP_LIM;
	free_vm->next = 0;
	free_vm->prev = 0;

	return;
}

struct page_directory *pd_create(void)
{
	struct page_directory *pd;
	u32 *pdir;
	int i;

	pd = (struct page_directory *) kmalloc(sizeof(struct page_directory));
	pd->base = get_page_from_heap();

	pdir = (u32 *) pd->base->v_addr;
	for (i = 0; i < 256; i++)
		pdir[i] = pd0[i];

	for (i = 256; i < 1023; i++)
		pdir[i] = 0;

	pdir[1023] = ((u32) pd->base->p_addr | (PG_PRESENT | PG_WRITE));
 
	pd->pt = 0;

	return pd;
}

int pd_destroy(struct page_directory *pd)
{
	struct page_list *pgh, *oldpgh;

	release_page_from_heap(pd->base->v_addr);

	pgh = pd->pt;

	while (pgh) {
		release_page_from_heap(pgh->page->v_addr);
		oldpgh = pgh;
		pgh = pgh->next;
		kfree(oldpgh);
	}

	kfree(pd);

	return 1;
}

int pd0_add_page(char *v_addr, char *p_addr, int flags)
{
	u32 *pde;
	u32 *pte;

	if (v_addr > (char *) USER_OFFSET) {
		printk("ERROR: pd0_add_page(): %p is not in kernel space !\n", v_addr);
		return 0;
	}

	pde = (u32 *) (0xFFFFF000 | (((u32) v_addr & 0xFFC00000) >> 20));
	if ((*pde & PG_PRESENT) == 0) {
		printk("PANIC: pd0_add_page(): kernel page table not found for v_addr %p. System halted !\n", v_addr);
		asm("hlt");
	}

	pte = (u32 *) (0xFFC00000 | (((u32) v_addr & 0xFFFFF000) >> 10));
	*pte = ((u32) p_addr) | (PG_PRESENT | PG_WRITE | flags);

	return 0;
}

int pd_add_page(char *v_addr, char *p_addr, int flags, struct page_directory *pd)
{
	u32 *pde;	
	u32 *pte;		
	u32 *pt;	
	struct page *newpg;
	struct page_list *pglist;
	int i;

	pde = (u32 *) (0xFFFFF000 | (((u32) v_addr & 0xFFC00000) >> 20));

	if ((*pde & PG_PRESENT) == 0) {

		newpg = get_page_from_heap();

		pt = (u32 *) newpg->v_addr;
		for (i = 1; i < 1024; i++)
			pt[i] = 0;

		*pde = (u32) newpg->p_addr | (PG_PRESENT | PG_WRITE | flags);

		if (pd) {
			if (pd->pt) {
				pglist = (struct page_list *) kmalloc(sizeof(struct page_list));
				pglist->page = newpg;
				pglist->next = pd->pt;
				pglist->prev = 0;
				pd->pt->prev = pglist;
				pd->pt = pglist;
			} else {
				pd->pt = (struct page_list *) kmalloc(sizeof(struct page_list));
				pd->pt->page = newpg;
				pd->pt->next = 0;
				pd->pt->prev = 0;
			}
		}

	}

	pte = (u32 *) (0xFFC00000 | (((u32) v_addr & 0xFFFFF000) >> 10));
	*pte = ((u32) p_addr) | (PG_PRESENT | PG_WRITE | flags);

	return 0;
}

int pd_remove_page(char *v_addr)
{
	u32 *pte;

	if (get_p_addr(v_addr)) {
		pte = (u32 *) (0xFFC00000 | (((u32) v_addr & 0xFFFFF000) >> 10));
		*pte = (*pte & (~PG_PRESENT));
		asm("invlpg %0"::"m"(v_addr));
	}

	return 0;
}

char *get_p_addr(char *v_addr)
{
	u32 *pde;		
	u32 *pte;		

	pde = (u32 *) (0xFFFFF000 | (((u32) v_addr & 0xFFC00000) >> 20));
	if ((*pde & PG_PRESENT)) {
		pte = (u32 *) (0xFFC00000 | (((u32) v_addr & 0xFFFFF000) >> 10));
		if ((*pte & PG_PRESENT))
			return (char *) ((*pte & 0xFFFFF000) + (VADDR_PG_OFFSET((u32) v_addr)));
	}

	return 0;
}