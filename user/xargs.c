#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

char **crateArr(int argc,char *argv[]){
    char **fargs;
    fargs = (char**)malloc(sizeof(char *)*MAXARG);
    for(int i = 0;i<MAXARG;++i){
        fargs[i] = (char*)malloc(512);
    }
    for(int i = 1;i<argc;++i)
        strcpy(fargs[i-1],argv[i]);
    return fargs;
}
void delArr(char **arr){
    for(int i = 0;i<MAXARG;++i)
        free(arr[i]);
    free(arr);
}

int
main(int argc,char *argv[]){
    if(argc<2){
        fprintf(2,"xargs <cmd> ...\n");
        exit(1);
    }

    char ch;
    char **args = crateArr(argc,argv);
    int argsPrt = argc-1;
    int stringPtr = 0;
    while(read(0,&ch,1)==1){

        //start prog
        if(ch=='\n'){
            if(fork()==0){
                args[argsPrt][stringPtr++] = '\0';
                args[++argsPrt] = 0;
                exec(argv[1],args);
            }
            wait(&argsPrt);
            argsPrt = argc-1;
            stringPtr = 0;
            continue;
        }

        //next arg
        if(ch==' '){
            args[argsPrt][stringPtr++] = '\0';
            ++argsPrt;
            if(argsPrt==MAXARG){
                fprintf(2,"args too long\n");
                exit(1);
            }
            stringPtr = 0;
            continue;
        }
        //combine ch
        args[argsPrt][stringPtr++] = ch;
    }
    delArr(args);
    
    exit(0);
}