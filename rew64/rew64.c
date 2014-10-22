#include <stdio.h>
#include <string.h>

#include "rew64.h"
#include "run.h"

bool read_asm;
// bool read_bin;

char *asm_filename;
// char bin_filename[200];

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
		}/* else if(streq(argv[i], "-b")){
			read_bin = true;
			++i;
			if(i < argc){
				bin_filename = argv[i];
			}
		}*/
	}
	/*
	if(read_asm == read_bin){
		return -1;
	}
	*/

	if(read_asm){
		// TODO: implement
	}/* else if(read_bin){
		// TODO: implement
		// load_binary(
	}*/
	puts("SUCCESS");
	return 0;
}
