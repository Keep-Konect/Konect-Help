#include "../io/io.h"
#include "../../screen/screen.h"
#include "../../types/types.h"
#include "keyboards_table.h"

#define __KBD__

#include "keyboard.h"

keyboard_status = 0;
caps = 0;

// disables the keyboard
void kkybrd_disable(){

	kybrd_ctrl_send_cmd(KYBRD_CTRL_CMD_DISABLE);
	keyboard_status = 0;
}

// enables the keyboard
void kkybrd_enable(){

	kybrd_ctrl_send_cmd(KYBRD_CTRL_CMD_ENABLE);
	keyboard_status = 1;
}

// read status from keyboard controller
int kybrd_ctrl_read_status(){

	return inb(KYBRD_CTRL_STATS_REG);
}

int kybrd_enc_read_buf(){

	return inb(KYBRD_ENC_INPUT_BUF);
}

// send command byte to keyboard controller
void kybrd_ctrl_send_cmd(int cmd) {

	// wait for kkybrd controller input buffer to be clear
	kkybrd_clear_buffer();

	outb(KYBRD_CTRL_CMD_REG, cmd);
}

// send command byte to keyboard encoder
void kybrd_enc_send_cmd(int cmd){

	// wait for kkybrd controller input buffer to be clear
	kkybrd_clear_buffer();

	// send command byte to kybrd encoder
	outb(KYBRD_ENC_CMD_REG, cmd);
}

void kkybrd_set_leds(int num, int caps, int scroll){

	int data = 0;

	// set or clear the bit
	data = scroll*1 + num*2 + caps*4;

	// send the command -- update keyboard Light Emetting Diods (LEDs)
	kybrd_enc_send_cmd (KYBRD_ENC_CMD_SET_LED);
	kybrd_enc_send_cmd (data);
}

// reset the system
void kkybrd_reset_system(){

	kybrd_ctrl_send_cmd (KYBRD_CTRL_CMD_WRITE_OUT_PORT);
	kybrd_enc_send_cmd (0xfe);
}

void kkybrd_clear_buffer(){
	while (1){
		if ((kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_IN_BUF) == 0){
			break;
		}
	}
}

void onKeyClick(){
	uchar i;
    static int lshift_enable;
    static int rshift_enable;
    static int alt_enable;
    static int ctrl_enable;

    do{
        i = inb(0x64);
    }
    while ((i & 0x01) == 0);

    i = inb(0x60);
    i--;


    if (i < 0x80) {
        switch (i) {
	        case 0:
	            if(caps == 0){
	        		kkybrd_set_leds(1, 1, 0);
	        		caps = 1;
	        	}else{
	        		kkybrd_set_leds(1, 0, 0);
	        		caps = 0;
	        	}
	            break;
	        case 0x35:
	            rshift_enable = 1;
	            break;
	        case 0x1C:
	            ctrl_enable = 1;
	            break;
	        case 0x37:
	            alt_enable = 1;
	            break;
	        case 13:
              cli();
	            deletecar();
              sti();
	            break;
	        default:
	            putcar(azerty[i * 4 + (lshift_enable || rshift_enable)], kattr);
        }
    }else{
        i -= 0x80;
        switch (i) {
	        case 0x29:
	            lshift_enable = 0;
	            break;
	        case 0x35:
	            rshift_enable = 0;
	            break;
	        case 0x1C:
	            ctrl_enable = 0;
	            break;
	        case 0x37:
	            alt_enable = 0;
	            break;
        }
    }
}
