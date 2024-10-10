#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

char path[256];
char programeName[256];

void* myFunc(void*){
    // 使用 system() 来执行外部程序，保持主线程的控制权
    int ret = system(path);
    if (ret == -1) {
        perror("system() failed");
    }
    return NULL;
}

int main() {
    pthread_t myThread;

    getcwd(path,sizeof(path));
    strcat(path, "/");
    printf("Enter the name of executable file\n");
    scanf("%s",programeName);
    strcat(path,programeName);
    
    if (pthread_create(&myThread, NULL, myFunc, NULL) != 0) {
        perror("Failed to create thread");
        return 1;
    }


    pthread_join(myThread,NULL);

    return 0;
}
