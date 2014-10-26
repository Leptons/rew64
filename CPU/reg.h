#ifndef __H__
#define __H__

typedef unsigned char byte;
typedef unsigned short hword;
typedef int32 word;
typedef int64 dword;
// typedef uint128 qword;

extern word CR;
extern dword LR, CTR, XER;
extern mem_addr NIP;

#define R_LENGTH 32

extern dword R[R_LENGTH];

#define FPR_LENGTH 32

extern double *FPR;


// AltiVec
#define VR_LENGTH 32
#define VSR_LENGTH 64

/*
extern qword *VR;
extern qword *VSR;
*/

extern bool mode_64bit;

#endif // __H__
