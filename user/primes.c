#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define numSize 35

void primerExit(int fd[]){
	close(fd[1]);
	wait(&fd[1]);
	exit(0);
}
void primerProcess(int fd[]){
	if(fork()==0){
		int p,n;
		int fd_new[2];
		close(fd[1]);//close write pipe from father
		if(read(fd[0],&p,4)==0) exit(0);
		printf("prime %d\n",p);
		pipe(fd_new);//create pipe give child
		primerProcess(fd_new);
		close(fd_new[0]);//father close read pipe

		do{
			if(read(fd[0],&n,4)==0) primerExit(fd_new);
			if(n%p==0) continue;//p does divide n then continue
			write(fd_new[1],&n,4);
		}while(n!=numSize);

		primerExit(fd_new);//for first fork
	}
}

int
main(int argc,char *argv[]){
	int fd[2];
	pipe(fd);
	primerProcess(fd);
	close(fd[0]);
	for(int i = 2;i<=numSize;++i){
		write(fd[1],&i,4);
	}
	wait(&fd[0]);
	exit(0);	
}
