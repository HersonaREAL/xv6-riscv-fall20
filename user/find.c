#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void
find(char *curDir,const char *findname){
    char buf[512],*p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(curDir, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", curDir);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", curDir);
        close(fd);
        return;
    }

    if(st.type==T_FILE){
        fprintf(2,"find <dir> <filename>\n");
        close(fd);
        return;
    }else if(st.type==T_DIR){
        //dir length <=14
        if(strlen(curDir) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("find: DIR path too long\n");
            exit(1);
        }
        //process Dir str
        strcpy(buf, curDir);
        p = buf+strlen(buf);
        *p++ = '/';

        //read file of dir
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            //pass .. and .
            if(strcmp(de.name,"..")==0||strcmp(de.name,".")==0)
                continue;
            if(de.inum == 0)
                continue;

            //process file name
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = '\0';
            if(stat(buf, &st) < 0){
                printf("find: cannot stat %s\n", buf);
                continue;
            }

            //is dir then rescure
            if(st.type==T_DIR)
                find(buf,findname);
            
            //find it ?
            if(strcmp(de.name,findname)==0){
                printf("%s\n",buf);
                continue;
            }
        }
        
    }else{
        fprintf(2,"what do you input?????????????????????????????????\n");
        exit(1);
    }
    close(fd);
}


int
main(int argc,char *argv[]){
    if(argc<3){
        fprintf(2,"find <dir> <filename>\n");
        exit(1);
    }

    find(".",argv[2]);
    exit(0);
}