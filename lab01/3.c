#include <stdio.h>
#include <pthread.h>
#include <semaphore.h> 

sem_t sem_arr[4];

void* Func_1(void*){
    sem_wait(&sem_arr[0]);
    printf("This ");
    sem_post(&sem_arr[1]);
    return NULL;
}
void* Func_2(void*){
    sem_wait(&sem_arr[1]);
    printf("is ");
    sem_post(&sem_arr[2]);
    return NULL;
}
void* Func_3(void*){
    sem_wait(&sem_arr[2]);
    printf("Jinan ");
    sem_post(&sem_arr[3]);
    return NULL;
}
void* Func_4(void*){
    sem_wait(&sem_arr[3]);
    printf("University!\n");
    return NULL;
}
int main() {
    pthread_t threads[4];
    int n;

    sem_init(&sem_arr[0],0,1);
    sem_init(&sem_arr[1],0,0);
    sem_init(&sem_arr[2],0,0);
    sem_init(&sem_arr[3],0,0);

    printf("Enter a number to loop execute threads\n");
    scanf("%d",&n);

    while (n--){
        pthread_create(&threads[0],NULL,Func_1,NULL);
        pthread_create(&threads[1],NULL,Func_2,NULL);
        pthread_create(&threads[2],NULL,Func_3,NULL);
        pthread_create(&threads[3],NULL,Func_4,NULL);

        for (int i=0;i<4;i++){
            pthread_join(threads[i],NULL);
        }

        sem_init(&sem_arr[0],0,1);
        sem_init(&sem_arr[1],0,0);
        sem_init(&sem_arr[2],0,0);
        sem_init(&sem_arr[3],0,0);
    }

    for (int i=0;i<4;i++){
        sem_destroy(&sem_arr[i]);
    }
    
    return 0;
}
