#ifndef __MEM_H__
#define __MEM_H__

typedef unsigned char byte;

extern instruction **text_seg;
#define TEXT_BOT ((mem_addr) 0x00000000)
extern mem_addr text_top;

extern byte *data_seg;
#define DATA_BOT ((mem_addr) 0x10000000)
extern mem_addr data_top;

extern byte *stack_seg;
extern mem_addr stack_bot;
#define STACK_TOP ((mem_addr) 0x70000000)

/*
extern byte *mem;
extern mem_addr mem_top;
extern mem_addr mem_bot;
*/

#define INST_PER_BYTE sizeof(instruction)
#define WORD_PER_BYTE 4
#define DWORD_PER_BYTE 8

#define IN_TEXT(X, N) (TEXT_BOT<=(X) && (X)+(N)<=text_top)
#define IN_DATA(X, N) (DATA_BOT<=(X) && (X)+(N)<=data_top)
#define IN_STACK(X, N) (stack_bot<=(X) && (X)+(N)<=STACK_TOP)

void make_memory(int text_size, int data_size, int stack_size);

instruction* read_mem_inst(mem_addr addr);
byte read_mem_8(mem_addr addr);
hword read_mem_16(mem_addr addr);
word read_mem_32(mem_addr addr);
dword read_mem_64(mem_addr addr);

void set_mem_inst(mem_addr addr, instruction *inst);
void set_mem_8(mem_addr addr, dword val);
void set_mem_16(mem_addr addr, dword val);
void set_mem_32(mem_addr addr, dword val);
void set_mem_64(mem_addr addr, dword val);

int get_CR(int pos);

#endif // __MEM_H__
