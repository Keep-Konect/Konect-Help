#include "../types/types.h"
#include "../lib/lib.h"

#define __NeedMM__
#include "../mm/mm.h"

#include "kmalloc.h"

void *ksbrk(int n)
{
	struct kmalloc_header *chunk;
	char *p_addr;
	int i;

	if ((kern_heap + (n * PAGESIZE)) > (char *) KERN_HEAP_LIM) {
		printk("PANIC: ksbrk(): no virtual memory left for kernel heap !\n");
		return (char *) -1;
	}

	chunk = (struct kmalloc_header *) kern_heap;

	//allocation of free memory
	for (i = 0; i < n; i++) {
		p_addr = get_page_frame();
		if (p_addr < 0) {
			printk("PANIC: ksbrk(): no free page frame available !\n");
			return (char *) -1;
		}

		pd0_add_page(kern_heap, p_addr, 0);

		kern_heap += PAGESIZE;
	}

	chunk->size = PAGESIZE * n;
	chunk->used = 0;

	return chunk;
}

void *kmalloc(unsigned long size)
{
	unsigned long realsize;	
	struct kmalloc_header *chunk, *other;

	if ((realsize =
	     sizeof(struct kmalloc_header) + size) < KMALLOC_MINSIZE)
		realsize = KMALLOC_MINSIZE;

	//search memory free
	chunk = (struct kmalloc_header *) KERN_HEAP;
	while (chunk->used || chunk->size < realsize) {
		if (chunk->size == 0) {
			printk
			    ("PANIC: kmalloc(): corrupted chunk on %x with null size (heap %x) !\nSystem halted\n",
			     chunk, kern_heap);
			asm("hlt");
		}

		chunk =
		    (struct kmalloc_header *) ((char *) chunk + chunk->size);

		if (chunk == (struct kmalloc_header *) kern_heap) {
			if (ksbrk((realsize / PAGESIZE) + 1) < 0) {
				printk("PANIC: kmalloc(): no memory left for kernel !\nSystem halted\n");
				asm("hlt");
			}
		} else if (chunk > (struct kmalloc_header *) kern_heap) {
			printf("PANIC: kmalloc(): chunk on %x while heap limit is on %x !\nSystem halted\n", chunk, kern_heap);
			asm("hlt");
		}
	}

	if (chunk->size - realsize < KMALLOC_MINSIZE)
		chunk->used = 1;
	else {
		other =
		    (struct kmalloc_header *) ((char *) chunk + realsize);
		other->size = chunk->size - realsize;
		other->used = 0;

		chunk->size = realsize;
		chunk->used = 1;
	}

	//return pointer to the memory
	return (char *) chunk + sizeof(struct kmalloc_header);
}

void kfree(void *v_addr)
{
	struct kmalloc_header *chunk, *other;

	//free bloc 
	chunk = (struct kmalloc_header *) (v_addr - sizeof(struct kmalloc_header));
	chunk->used = 0;

	while ((other = (struct kmalloc_header *) ((char *) chunk + chunk->size)) && other < (struct kmalloc_header *) kern_heap && other->used == 0){
		chunk->size += other->size;
	}
		
}
