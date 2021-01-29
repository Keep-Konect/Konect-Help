void sti();
void cli();
int inb(int);
void outbp(int, int);
void outb(int, int);

#ifdef __io__
    typedef struct RegisterAsm{
        char ax, bx, cx, dx;
        char si, di;
        char sp;
        char bp;
        char cs, ds, es, ss;
        char ip;
        char flags;
    }RegisterAsm;
    //extern void callBIOSInRealMode();
#else
    extern struct RegisterAsm;
#endif

extern void go16();

