#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

char path[256];
char programeName[256];

int main() {
    getcwd(path,sizeof(path));
    strcat(path, "/");
    printf("Enter the name of executable file\n");
    scanf("%s",programeName);
    strcat(path,programeName);

    pid_t pid = fork();

    if (pid<0){
        //fork failed
        perror("fork failed");
        exit(1);
    }else if(pid==0){
        execlp(path,programeName,(char*)NULL);
        perror("execlp failed");
        exit(1);
    } else{
        wait(NULL);
        printf("over.\n");
    }

    return 0;
}
