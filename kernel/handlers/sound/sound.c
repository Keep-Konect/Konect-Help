#include "sound.h"

void play_sound(int nFrequence){
 	int Div;
 	int tmp;
 
    //Set the PIT to the desired frequency
 	Div = 1193180 / nFrequence;
 	outb(0x43, 0xb6);
 	outb(0x42, (int) (Div) );
 	outb(0x42, (int) (Div >> 8));
 
 	tmp = inb(0x61);
  	if (tmp != (tmp | 3)){
 		outb(0x61, tmp | 3);
 	}
}
 
void nosound(){
 	int tmp = inb(0x61) & 0xFC;
 
 	outb(0x61, tmp);
}
