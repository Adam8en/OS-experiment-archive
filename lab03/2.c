#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_PROCESSES 10

struct PCB {
    char pid[64];
    char state;
    int hasFinished;
    int priority;
    int neededTime;
    int totalWaitTime;
    int arrivalTime;
    double responseRatio;

    struct PCB *next; 
};

// 随机生成进程
void generateProcesses(struct PCB processes[]) {
    srand(time(0));  
    for (int i = 0; i < NUM_PROCESSES; i++) {
        snprintf(processes[i].pid, sizeof(processes[i].pid), "P%d", i + 1);
        processes[i].state = 'w';
        processes[i].hasFinished=0;
        processes[i].priority = 0;
        processes[i].neededTime = (rand() % 41) + 10;  // 随机生成需要的时间1~50
        processes[i].arrivalTime = (rand() % 20) + 1;  // 随机生成抵达时间1~20
        processes[i].totalWaitTime = 0;
        processes[i].responseRatio = 0;
    }
}

// 计算响应比
void calculateResponseRatio(struct PCB processes[], int currentTime){
    int waitTime=0;
    for (int i=0;i<NUM_PROCESSES;i++){
        if (processes[i].hasFinished!=1&&processes[i].arrivalTime <= currentTime){
            waitTime=currentTime-processes[i].arrivalTime;
            // 计算公式：响应比 =（等待时间+运行时间）/运行时间
            processes[i].responseRatio=(waitTime+(double)processes[i].neededTime)/processes[i].neededTime;
        }
    }
    return;
}

// 寻找响应比最大的进程
int findHighestResponseProcess(struct PCB processes[],int currentTime){
    int index=-1;
    double highestRatio=-1;

    for (int i=0;i<NUM_PROCESSES;i++){
        if (processes[i].hasFinished!=1 && processes[i].responseRatio>highestRatio &&processes[i].arrivalTime<=currentTime){
            index=i;
            highestRatio=processes[i].responseRatio;
        }
    }

    return index;
}

// 模拟HRRN调度排序器
void HRRN(struct PCB processes[]) {
    int totalWaitTime=0;
    int currentTime=0;
    int finishedProcesses=0;
    int index=0;

    printf("Execution Order:\n");

    while(finishedProcesses<NUM_PROCESSES){

        calculateResponseRatio(processes,currentTime); // 计算响应比

        index=findHighestResponseProcess(processes,currentTime); // 寻找响应值最大的进程下标
        if (index==-1)
        {
            currentTime++;
            continue;
        }

        struct PCB* tmp=&processes[index];
        printf("Process %s - Arrival Time: %d, Needed Time: %d, Wait Time: %d\n",
        tmp->pid, tmp->arrivalTime, tmp->neededTime,
        currentTime-tmp->arrivalTime);

        // 计算总共等待的时间
        tmp->totalWaitTime=currentTime-tmp->arrivalTime;
        totalWaitTime+=tmp->totalWaitTime;
        
        // 更新进程状态和时间
        tmp->hasFinished=1;
        currentTime+=tmp->neededTime;
        finishedProcesses++;
    }

    // 计算等待平均时间
    double avgWaitTime = (double)totalWaitTime / NUM_PROCESSES;
    printf("Total Wait Time: %d\n", totalWaitTime);
    printf("Average Wait Time: %.2f\n", avgWaitTime);
}

int main() {
    struct PCB processes[NUM_PROCESSES];
    
    generateProcesses(processes);

    HRRN(processes);

    return 0;
}
