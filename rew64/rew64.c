#include <stdio.h>
#include <string.h>

#include "rew64.h"
#include "run.h"

bool read_asm;

char *asm_filename;

int main(int argc, char *argv[]){
	int i;

	for(i = 1; i < argc; i++){
		if(streq(argv[i], "-help")){
			printf("\n"); // TODO
		} else if(streq(argv[i], "-f")){
			read_asm = true;
			++i;
			if(i < argc){
				asm_filename = argv[i];
			}
		}
	}


	if(read_asm){
		// TODO: implement
	}
	puts("SUCCESS");
	return 0;
}
