#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define PROCESS_NUM 5 // n
#define RESOURCE_NUM 3 // m

int Available[RESOURCE_NUM];
int Max[PROCESS_NUM][RESOURCE_NUM];
int Allocation[PROCESS_NUM][RESOURCE_NUM];
int Need[PROCESS_NUM][RESOURCE_NUM];
int SafeSequence[PROCESS_NUM];

void init(void);
int is_safe(void);
void banker(void);

int main(void){
    init();

    banker();

    return 0;
}

void init(void){
    srand((unsigned)time(NULL));

    // Generate available resources
    printf("Initial available:\n");
    for (int i=0;i<RESOURCE_NUM;i++){
        Available[i] = rand()%10+1;
        printf("%d ",Available[i]);
    }
    printf("\n\n");

    // Generate max need of each process
    printf("Max:\n");
    for (int i=0;i<PROCESS_NUM;i++){
        printf("P%d: ",i);
        for (int j=0;j<RESOURCE_NUM;j++){
            Max[i][j]=rand()%Available[j]+1;
            printf("%d ",Max[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    // Generate initial allocated matrix
    printf("Allocated:\n");
    int tmp_allocated[RESOURCE_NUM]={0};
    for(int i=0;i<PROCESS_NUM;i++){
        printf("P%d: ",i);
        for (int j=0;j<RESOURCE_NUM;j++){
            if (tmp_allocated[j]<Available[j]){
                int rand_num=rand()%2;
                if (rand_num==1){
                    int allocated_resource = rand()%Max[i][j]+1;
                    while (tmp_allocated[j]+allocated_resource>Available[j])
                        allocated_resource--;
                    Allocation[i][j]=allocated_resource;
                    tmp_allocated[j]+=allocated_resource;
                }else Allocation[i][j]=0;
            }else{
                Allocation[i][j]=0;
            }
            printf("%d ",Allocation[i][j]);
        }
        printf("\n");
    }

    // Generate Need matrix
    for (int i=0;i<PROCESS_NUM;i++){
        for (int j=0;j<RESOURCE_NUM;j++){
            Need[i][j]=Max[i][j]-Allocation[i][j];
        }
    }

    // Calculate Available
    printf("Available:\n");
    for (int i=0;i<RESOURCE_NUM;i++){
        Available[i]-=tmp_allocated[i];
        printf("%d ",Available[i]);
    }
    printf("\n\n");
}

int is_safe(void){
    int Work[RESOURCE_NUM];
    int Finish[PROCESS_NUM]={0};
    int index=0;

    // Load and Display Work
    printf("Work:\n");
    for (int i=0;i<RESOURCE_NUM;i++){
        Work[i]=Available[i];
        printf("%d ",Work[i]);
    }
    printf("\n\n");

    //Dispaly Need
    printf("Need:\n");
    for (int i=0;i<PROCESS_NUM;i++){
        printf("P%d: ",i);
        for (int j=0;j<RESOURCE_NUM;j++){
            printf("%d ",Need[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    int found_process = 0;
    while(found_process!=PROCESS_NUM){
        /*
        try to find a process which satisfy:
            a. Finish[i] == false
            b. Need_i <= Work 
        */
        found_process = 0;
        for (int i=0;i<PROCESS_NUM;i++){
            // check if Finish[i] == false
            if(Finish[i]==0){
                // check if Work < Need
                int NeedBiggerThanWork = 0;
                for (int j=0;j<RESOURCE_NUM;j++){
                    if (Need[i][j]>Work[j]){
                        NeedBiggerThanWork++;
                    }
                }
                if (NeedBiggerThanWork){
                    // Need_i > Work, continue finding
                    found_process++;
                    continue;
                }else{
                    // Work = Work + Allocation_i
                    for (int j=0;j<RESOURCE_NUM;j++){
                        Work[j]+=Allocation[i][j];
                    }

                    Finish[i]=1;
                    SafeSequence[index++]=i;
                    // return to find process
                    break;
                }
            }else{
                // Finish[i] == true
                found_process++;
            }
        }
    }

    // Check Finish
    int finish=0;
    for (int i=0;i<PROCESS_NUM;i++){
        if(Finish[i]==0) finish++;
    }
    if(finish==0)
        return 1; // is safe
    else
        return 0; // not safe
}

void banker(void){
    //编程实现银行家算法，检测从初始分配开始，是否存在安全分配序列。如果存在，刚输出该安全分配序列，否则输出“Deadlock”。
    if (is_safe()){
        printf("Safe Sequence: ");
        for (int i=0;i<PROCESS_NUM;i++){
            printf("P%d ",SafeSequence[i]);
        }
        printf("\n");
    }else{
        printf("Deadlock!\n");
    }
}