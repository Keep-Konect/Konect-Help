#include "../types/types.h"

#define	PAGESIZE 	4096
#define	RAM_MAXSIZE	0x100000000
#define	RAM_MAXPAGE	0x100000

#define IDTSIZE		0xFF	
#define GDTSIZE		0xFF

#define IDTBASE		0x00000000
#define GDTBASE		0x00000800

#define	KERN_PDIR		0x00001000
#define	KERN_STACK		0x0009FFF0
#define	KERN_BASE		0x00100000
#define KERN_PG_HEAP		0x00800000
#define KERN_PG_HEAP_LIM	0x10000000
#define KERN_HEAP		0x10000000
#define KERN_HEAP_LIM		0x40000000

#define	USER_OFFSET 		0x40000000
#define	USER_STACK 		0xE0000000

#define	VADDR_PD_OFFSET(addr)	((addr) & 0xFFC00000) >> 22
#define	VADDR_PT_OFFSET(addr)	((addr) & 0x003FF000) >> 12
#define	VADDR_PG_OFFSET(addr)	(addr) & 0x00000FFF
#define PAGE(addr)		(addr) >> 12

#define	PAGING_FLAG 	0x80000000
#define PSE_FLAG	0x00000010

#define PG_PRESENT	0x00000001	
#define PG_WRITE	0x00000002
#define PG_USER		0x00000004
#define PG_4MB		0x00000080


#ifndef __MM_STRUCT__
#define __MM_STRUCT__

struct page {
	char *v_addr;
	char *p_addr;
};

struct page_list {
	struct page *page;
	struct page_list *next;
	struct page_list *prev;
};

struct page_directory {
	struct page *base;
	struct page_list *pt;
};

struct vm_area {
	char *vm_start;	
	char *vm_end;	
	struct vm_area *next;
	struct vm_area *prev;
};

#endif

#ifdef __NeedMM__
	extern char *kern_heap;
	extern struct vm_area *free_vm;
	extern u32 *pd0;
#endif

#ifdef __MM__
	u32 *pd0 = (u32 *) KERN_PDIR;
	char *pg0 = (char *) 0;	
	char *pg1 = (char *) 0x400000;	
	char *pg1_end = (char *) 0x800000;
	u8 mem_bitmap[RAM_MAXPAGE / 8];
	char *kern_heap;
	struct vm_area *free_vm;
#else
	extern u8 mem_bitmap[];
#endif

char *get_page_frame(void);

void set_page_frame_used(int);

void release_page_frame(int);

struct page *get_page_from_heap(void);
int release_page_from_heap(char *);

void init_mm(u32);

struct page_directory *pd_create(void);
int pd_destroy(struct page_directory *);

int pd0_add_page(char *, char *, int);

int pd_add_page(char *, char *, int, struct page_directory *);
int pd_remove_page(char *);

char *get_p_addr(char *);
