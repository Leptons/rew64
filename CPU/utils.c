#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rew64.h"
#include "reg.h"
#include "inst.h"
#include "mem.h"
#include "utils.h"


// e.g. rev(0b00110101, 2, 4) = 0b01010011
int64 rev(int64 val, int n, int size){
	int64 mask = (1LL << size) - 1;
	int i;

	int64 res = 0;
	for(i = 0; i < n; i++){
		res += (val & mask) << ((n-i-1) * size);
		val >>= size;
	}

	return res;
}

void *xrealloc(void *mem, int size){
	if(mem == NULL){
		return mem = malloc(size);
	} else {
		return mem = realloc(mem, size);
	}
}

void *xmalloc(int size){
	void *x = (void*) malloc(size);
	if(x==0){
		fprintf(stderr, "Cannot allocate memory\n");
	}
	return x;
}

void *zmalloc(int size){
	void *z = xmalloc(size);
	memset(z, 0, size);
	return z;
}
