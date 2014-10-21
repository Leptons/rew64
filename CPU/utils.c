#include <stdlib.h>
#include <string.h>

#include "rew64.h"
#include "utils.h"

void *xrealloc(void *mem, int size){
	if(mem == NULL){
		return mem = malloc(size);
	} else {
		return mem = realloc(mem, size);
	}
}

void *xmalloc(int size){
	void *x = (void*) malloc(size);
	if(x==0); // TODO: error
	return x;
}

void *zmalloc(int size){
	void *z = xmalloc(size);
	memset(z, 0, size);
	return z;
}
