#include <stdio.h>
#include <stdlib.h>

#include "rew64.h"
#include "utils.h"
#include "reg.h"
#include "inst.h"
#include "mem.h"


// registers
word CR;
dword LR, CTR, XER;
mem_addr NIP;
dword GPR[GPR_LENGTH];
double *FPR;
/*
qword *VR;
qword *VSR;
*/

bool mode_64bit = true;

// storage
instruction **text_seg;
mem_addr text_top;
byte *data_seg;
mem_addr data_top;
byte *stack_seg;
mem_addr stack_bot;


static void free_instructions(instruction **inst, int n);
static instruction *bad_inst_read(mem_addr addr);
static void bad_inst_write(mem_addr);
static byte bad_read(mem_addr addr);
static void bad_write(mem_addr addr);


void make_memory(int text_size, int data_size, int stack_size){
	text_size = ROUND_UP(text_size, WORD_PER_BYTE);
	if(text_seg != NULL){
		free_instructions(text_seg, (text_top - TEXT_BOT) / 4);
	}
	text_top = TEXT_BOT + text_size;
	text_seg = (instruction**) xrealloc(text_seg, text_size * sizeof(instruction*));

	data_size = ROUND_UP(data_size, DWORD_PER_BYTE);
	data_top = DATA_BOT + data_size;
	data_seg = (byte*) xrealloc(data_seg, data_size);

	stack_size = ROUND_UP(stack_size, DWORD_PER_BYTE);
	stack_bot = STACK_TOP - stack_size;
	stack_seg = (byte*) xrealloc(stack_seg, stack_size);

}

instruction* read_mem_inst(mem_addr addr){
	if(IN_TEXT(addr, 4) && !(addr&0x3)){
		return text_seg[addr - TEXT_BOT];
	} else {
		return bad_inst_read(addr);
	}
}

byte read_mem_8(mem_addr addr){
	byte *mem;
	if(IN_DATA(addr, 1)){
		mem = data_seg - DATA_BOT;
	} else if(IN_STACK(addr, 1)){
		mem = stack_seg - stack_bot;
	} else {
		return bad_read(addr);
	}

	return mem[addr];
}

hword read_mem_16(mem_addr addr){
	byte *mem;
	if(IN_DATA(addr, 2)){
		mem = data_seg - DATA_BOT;
	} else if(IN_STACK(addr, 2)){
		mem = stack_seg - stack_bot;
	} else {
		return bad_read(addr);
	}

	return (
			(hword)(mem[addr+0] << 0x04) |
			(hword)(mem[addr+1] << 0x00));
}

word read_mem_32(mem_addr addr){
	byte *mem;
	if(IN_DATA(addr, 4)){
		mem = data_seg - DATA_BOT;
	} else if(IN_STACK(addr, 4)){
		mem = stack_seg - stack_bot;
	} else {
		return bad_read(addr);
	}

	return (
			(word)(mem[addr+0] << 0x0c) |
			(word)(mem[addr+1] << 0x08) |
			(word)(mem[addr+2] << 0x04) |
			(word)(mem[addr+3] << 0x00));
}

dword read_mem_64(mem_addr addr){
	byte *mem;
	if(IN_DATA(addr, 8)){
		mem = data_seg - DATA_BOT;
	} else if(IN_STACK(addr, 8)){
		mem = stack_seg - stack_bot;
	} else {
		return bad_read(addr);
	}

	return (
			(word)(mem[addr+0] << 0x1c) |
			(word)(mem[addr+1] << 0x18) |
			(word)(mem[addr+2] << 0x14) |
			(word)(mem[addr+3] << 0x10) |
			(word)(mem[addr+4] << 0x0c) |
			(word)(mem[addr+5] << 0x08) |
			(word)(mem[addr+6] << 0x04) |
			(word)(mem[addr+7] << 0x00));
}

void set_mem_inst(mem_addr addr, instruction *inst){
	if(IN_TEXT(addr, 1)){
		text_seg[addr - TEXT_BOT] = inst;
	} else {
		bad_inst_write(addr);
	}
}

void set_mem_8(mem_addr addr, byte val){
	byte *mem;
	if(IN_DATA(addr, 1)){
		mem = data_seg - DATA_BOT;
	} else if(IN_STACK(addr, 1)){
		mem = stack_seg - stack_bot;
	} else {
		bad_write(addr);
	}

	mem[addr] = val;
}

void set_mem_16(mem_addr addr, hword val){
	byte *mem;
	if(IN_DATA(addr, 2)){
		mem = data_seg - DATA_BOT;
	} else if(IN_STACK(addr, 2)){
		mem = stack_seg - stack_bot;
	} else {
		bad_write(addr);
	}

	mem[addr+0] = (byte)((val >> 0x04) & 0xff);
	mem[addr+1] = (byte)((val >> 0x00) & 0xff);
}

void set_mem_32(mem_addr addr, word val){
	byte *mem;
	if(IN_DATA(addr, 2)){
		mem = data_seg - DATA_BOT;
	} else if(IN_STACK(addr, 2)){
		mem = stack_seg - stack_bot;
	} else {
		bad_write(addr);
	}

	mem[addr+0] = (byte)((val >> 0x0c) & 0xff);
	mem[addr+1] = (byte)((val >> 0x08) & 0xff);
	mem[addr+2] = (byte)((val >> 0x04) & 0xff);
	mem[addr+3] = (byte)((val >> 0x00) & 0xff);
}

void set_mem_64(mem_addr addr, dword val){
	byte *mem;
	if(IN_DATA(addr, 2)){
		mem = data_seg - DATA_BOT;
	} else if(IN_STACK(addr, 2)){
		mem = stack_seg - stack_bot;
	} else {
		bad_write(addr);
	}

	mem[addr+0] = (byte)((val >> 0x1c) & 0xff);
	mem[addr+1] = (byte)((val >> 0x18) & 0xff);
	mem[addr+2] = (byte)((val >> 0x14) & 0xff);
	mem[addr+3] = (byte)((val >> 0x10) & 0xff);
	mem[addr+4] = (byte)((val >> 0x0c) & 0xff);
	mem[addr+5] = (byte)((val >> 0x08) & 0xff);
	mem[addr+6] = (byte)((val >> 0x04) & 0xff);
	mem[addr+7] = (byte)((val >> 0x00) & 0xff);
}

static void free_instructions(instruction **inst, int n){
	for(; n; n--, inst++){
		if(*inst){
			free_inst(*inst);
		}
	}
}

static instruction *bad_inst_read(mem_addr addr){
	fprintf(stderr, "Cannot read text %016lx\n", (unsigned long)addr);
	return decode_inst(0);
}

static void bad_inst_write(mem_addr addr){
	fprintf(stderr, "Cannot write text %016lx\n", (unsigned long)addr);
}

static byte bad_read(mem_addr addr){
	fprintf(stderr, "Cannot read %016lx\n", (unsigned long)addr);
	return 0;
}

static void bad_write(mem_addr addr){
	fprintf(stderr, "Cannot write %016lx\n", (unsigned long)addr);
}
