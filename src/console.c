/*
 * console.c
 *
 *  Created on: 18.10.2022
 *      Author: maite
 */

#include <console.h>
#include <utils.h>

static char *vid_mem = (char*) 0xb8000;
static int pos_x, pos_y;

#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25

void kputs(char* c) {
	int i = 0;
	while(1) {
		if(c[i] == '\0')
			return;
		kputc(c[i]);
		i++;
	}
}

void kputc(char c) {
	// works in qemu, write to serial
	outb(0x3F8, c);

	if(c != '\n') {
		int index = pos_x + pos_y * SCREEN_WIDTH;
		vid_mem[index * 2] = c;
		vid_mem[index * 2 + 1] = 0x03;
		pos_x++;
	} else {
		outb(0x3F8, (char) '\r');
		pos_x = 0;
		pos_y++;
	}
	if(pos_x >= SCREEN_WIDTH) {
		pos_y++;
		pos_x = 0;
	}

	if(pos_y >= SCREEN_HEIGHT) {
		for(int i = SCREEN_WIDTH; i< SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
			vid_mem[(i - SCREEN_WIDTH) * 2] = vid_mem[i * 2];
			vid_mem[(i - SCREEN_WIDTH) * 2 + 1] = vid_mem[i * 2 + 1];
		}
		for(int i = SCREEN_WIDTH*(SCREEN_HEIGHT - 1); i<SCREEN_WIDTH*(SCREEN_HEIGHT); i++) {
			vid_mem[(i) * 2] = ' ';
			vid_mem[(i) * 2 + 1] = 0x03;
		}
		pos_y = SCREEN_HEIGHT - 1;
		pos_x = 0;

	}
}

void serial_write(char c) {

}
