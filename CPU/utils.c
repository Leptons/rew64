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

// rotate left n times (m bit)
int64 rot(int64 val, int n, int m){
	int i;
	m--;
	for(i = 0; i < n; i++){
		int tmp = (val>>m)&1;
		val <<= 1;
		val |= tmp;
	}

	return val;
}

// mask from left through right
int64 mk_mask(int left, int right){
	if(left > right) return 0;
	int64 mask = 0;
	if(left==0){
		mask |= 1LL<<63;
		left++;
	}
	left = 63 - left;
	right = 63 - right;
	mask |= (1LL<<(left+1)) - (1LL<<right);
	return mask;
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
