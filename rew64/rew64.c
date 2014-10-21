#include <stdio.h>
#include <string.h>

#include "rew64.h"
#include "run.h"

int main(int argc, char *argv[]){
	int i;

	for(i = 1; i < argc; i++){
		if(streq(argv[i], "-help")){
			printf("\n"); // TODO
		}
	}
	puts("SUCCESS");
	return 0;
}
