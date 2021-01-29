#ifdef __reg__
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
    int getAx();
    void setAx(int);
#endif