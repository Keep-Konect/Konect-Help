#include <stdarg.h>
#include "../types/types.h"
#include "../drivers/io/io.h"

#define __SCREEN__
#include "screen.h"

void putpixel(unsigned char* screen, int x, int y, int r, int g, int b) {
    int where = (x + (y * screenSizeX)) * 3;
    screen[where] = b;          // BLUE
    screen[where + 1] = g;      // GREEN
    screen[where + 2] = r;      // RED
}
void fillrect(unsigned char *vram, int w, int h, int x, int y, int r, int g, int b) {
    unsigned char *where = vram;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            putpixel(vram, j + x, i + y, r, g, b);
        }
    }
}
void fillscreen(unsigned char* screen, int r, int g, int b){
  fillrect(screen, screenSizeX, screenSizeY, 0, 0, r, g, b);
}
void clearscreen(unsigned char* screen){
  fillrect(screen, screenSizeX, screenSizeY, 0, 0, 0, 0, 0);
}


//for text mode only
void scrollup(unsigned int n)
{
    unsigned char *video, *tmp;

    for (video = (unsigned char *) RAMSCREEN; video < (unsigned char *) SCREENLIM; video += 2) {
        tmp = (unsigned char *) (video + n * 160);

        if(tmp < (unsigned char *) SCREENLIM){
            *video = *tmp;
            *(video + 1) = *(tmp + 1);
        }else{
            *video = 0;
            *(video + 1) = 0;
        }
    }

    CursorY -= n;
    if (CursorY < 0){
        CursorY = 0;
    }
    show_cursor();
}

void putcar(uchar c, char kat)
{
    unsigned char *video;

    if (c == 10){
        //CR-NL
        CursorX = 0;
        CursorY++;
    }else if (c == 9){
        //TAB
        CursorX = CursorX + 8 - (CursorX % 8);
    }else if (c == 13){
        //CR
        CursorX = 0;
    }else{
        //other characters
        video = (unsigned char *) RAMSCREEN;
        video += (unsigned int) 2 * CursorX + 160 * CursorY;
        *video = c;
        video += 1;
        *video = kat;

        CursorX++;
        if (CursorX > 79) {
            CursorX = 0;
            CursorY++;
        }
    }
    if (CursorY > 24){
        scrollup(CursorY - 24);
    }
    show_cursor();
}

void dump(uchar* addr, int n)
{
    char c1, c2;
    char *tab = "0123456789ABCDEF";

    while(n--) {
        c1 = tab[(*addr & 0xF0)>>4];
        c2 = tab[*addr & 0x0F];
        addr++;
        putcar(c1, 0x0F);
        putcar(c2, 0x0F);
    }
}

void deletecar(){
    unsigned char *video, *tmp;

    video = (unsigned char *) RAMSCREEN;
    video += (unsigned int) 2 * CursorX + 160 * CursorY;
    video -= 1;
    while(*video == 0){
        if(CursorX == 0){
            CursorY --;
            CursorX = 79;
        }else{
           CursorX --;
        }
        video = (unsigned char *) RAMSCREEN;
        video += (unsigned int) 2 * CursorX + 160 * CursorY;
        show_cursor();
    }

    if(CursorX == 0){
        CursorY --;
        CursorX = 79;
    }else{
        CursorX --;
    }
    video = (unsigned char *) RAMSCREEN;
    video += (unsigned int) 2 * CursorX + 160 * CursorY;
    *video = 0;
    show_cursor();
}

void print(char *string, char kat)
{
    while (*string != 0) {
        putcar(*string, kat);
        string++;
    }
}

void printf(char *string)
{
    print(string, kattr);
}

void move_cursor(int x, int y)
{
    int c_pos;
    unsigned char *video, *tmp;

    video = (unsigned char *) RAMSCREEN;
    video += (unsigned int) 2 * CursorX + 160 * CursorY;
    video -= 1;
    *tmp = *video;
    if(*video != 0){
        video += 2;
        *video = *tmp;
    }else{
        *video = kattr;
    }
    c_pos = y * 80 + x;
    outb(0x3d4, 0x0f);
    outb(0x3d5, (u8) c_pos);
    outb(0x3d4, 0x0e);
    outb(0x3d5, (u8) (c_pos >> 8));
}

void show_cursor()
{
    move_cursor(CursorX, CursorY);
}

void printk(char *s, ...)
{
    va_list ap;

    char buf[16];
    int i, j, size, buflen, neg;

    unsigned char c;
    int ival;
    unsigned int uival;

    va_start(ap, s);

    while ((c = *s++)) {
        size = 0;
        neg = 0;

        if (c == 0)
            break;
        else if (c == '%') {
            c = *s++;
            if (c >= '0' && c <= '9') {
                size = c - '0';
                c = *s++;
            }

            if (c == 'd') {
                ival = va_arg(ap, int);
                if (ival < 0) {
                    uival = 0 - ival;
                    neg++;
                } else
                    uival = ival;
                itoa(buf, uival, 10);

                buflen = strlen(buf);
                if (buflen < size)
                    for (i = size, j = buflen; i >= 0;
                         i--, j--)
                        buf[i] =
                            (j >=
                             0) ? buf[j] : '0';

                if (neg){
                    printk("-%s", buf);
                }
                else{
                    printk(buf);
                }
            } else if (c == 'u') {
                uival = va_arg(ap, int);
                itoa(buf, uival, 10);

                buflen = strlen(buf);
                if (buflen < size)
                    for (i = size, j = buflen; i >= 0;
                         i--, j--)
                        buf[i] =
                            (j >=
                             0) ? buf[j] : '0';

                printk(buf);
            } else if (c == 'x' || c == 'X') {
                uival = va_arg(ap, int);
                itoa(buf, uival, 16);

                buflen = strlen(buf);
                if (buflen < size)
                    for (i = size, j = buflen; i >= 0;
                         i--, j--)
                        buf[i] =
                            (j >=
                             0) ? buf[j] : '0';

                printk("0x%s", buf);
            } else if (c == 'p') {
                uival = va_arg(ap, int);
                itoa(buf, uival, 16);
                size = 8;

                buflen = strlen(buf);
                if (buflen < size)
                    for (i = size, j = buflen; i >= 0;
                         i--, j--)
                        buf[i] =
                            (j >=
                             0) ? buf[j] : '0';

                printk("0x%s", buf);
            }else if(c == 's') {
                printk((char *) va_arg(ap, int));
            } 
        }else
            putcar(c, 0x0E);
    }

    return;
}