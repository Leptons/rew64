#ifndef __H__
#define __H__

typedef unsigned char byte;
typedef unsigned short hword;
typedef uint32 word;
typedef uint64 dword;
typedef uint128 qword;


extern dword CR, LR, CTR, XER;

extern mem_addr NIP;

#define GPR_LENGTH 32

extern dword GPR[GPR_LENGTH];

#define FPR_LENGTH 32

extern double *FPR;

#define VR_LENGTH 32
#define VSR_LENGTH 64

extern qword *VR;
extern qword *VSR;

#endif // __H__
