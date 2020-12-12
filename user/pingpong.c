#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc,char *argv[]){
	//create two pipe one for child , the other for father
	int fd0[2],fd1[2];
	char *byte = "p";
	pipe(fd0);
	pipe(fd1);
	int fork_ret = fork();

	//check fork
	if(fork_ret<0){
		fprintf(2,"fork fail\n");
		exit(1);
	}
	if(fork_ret==0){
		close(fd0[1]);//close write
		close(fd1[0]);//close read
		if(read(fd0[0],byte,1)==1){
			fprintf(1,"%d: received ping\n",getpid());
			write(fd1[1],byte,1);
			exit(0);
		}else{
			fprintf(2,"read error\n");
			exit(1);
		}
	}

	//father
	close(fd0[0]);//close read
	close(fd1[1]);//close write
	if(write(fd0[1],byte,1)!=1){
		fprintf(2,"write error\n");
		exit(1);
	}
	if(read(fd1[0],byte,1)==1){
		fprintf(1,"%d: received pong\n",getpid());
	}
	exit(0);	
}
