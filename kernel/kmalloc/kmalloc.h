#include "../types/types.h"

#define KMALLOC_MINSIZE		16

struct kmalloc_header{
	unsigned long size:31;	//total size
	unsigned long used:1;
} __attribute__ ((packed));

void *ksbrk(int);
void *kmalloc(unsigned long);
void kfree(void *);