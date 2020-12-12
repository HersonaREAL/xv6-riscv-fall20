#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc,char *argv[]){
	if(argc<=1){
		fprintf(2,"Usage: sleep <tick>\n");
		exit(1);
	}
	int s = atoi(argv[1]);
	sleep(s);
	exit(0);	
}
