#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_PROCESSES 10

struct PCB {
    char pid[64];
    char state;
    int priority;
    int neededTime;
    int totalWaitTime;
    int arrivalTime;
    struct PCB *next; // 实际上并没用到指针，用数组代替了
};

// 随机生成进程
void generateProcesses(struct PCB processes[]) {
    srand(time(0));  
    for (int i = 0; i < NUM_PROCESSES; i++) {
        snprintf(processes[i].pid, sizeof(processes[i].pid), "P%d", i + 1);
        processes[i].state = 'w';
        processes[i].priority = 0;
        processes[i].neededTime = (rand() % 41) + 10;  // 随机生成需要的时间1~50
        processes[i].arrivalTime = (rand() % 20) + 1;  // 随机生成抵达时间1~20
        processes[i].totalWaitTime = 0;
    }
}

// 撰写SFJ排序器的比较逻辑
int compare(const void *a, const void *b) {
    struct PCB *p1 = (struct PCB *)a;
    struct PCB *p2 = (struct PCB *)b;

    // 优先比较需要用时，如果时间相同则比较到达时间
    if (p1->neededTime == p2->neededTime)
        return p1->arrivalTime - p2->arrivalTime;
    return p1->neededTime - p2->neededTime;
}

// 模拟SFJ调度排序器
void SJF(struct PCB processes[]) {
    // 对进程进行排序，运行时间少、抵达时间早的优先
    qsort(processes, NUM_PROCESSES, sizeof(struct PCB), compare);

    int currentTime = 0;
    int totalWaitTime = 0;

    printf("Execution Order:\n");

    for (int i = 0; i < NUM_PROCESSES; i++) {
        // 如果第一个进程的抵达时间晚于现在的时间，则等待第一个进程开始
        if (currentTime < processes[i].arrivalTime) {
            currentTime = processes[i].arrivalTime;
        }

        // 计算进程等待时间：当前的时间 - 抵达的时间
        processes[i].totalWaitTime = currentTime - processes[i].arrivalTime;
        // 计算等待总时间
        totalWaitTime += processes[i].totalWaitTime;

        printf("Process %s - Arrival Time: %d, Needed Time: %d, Wait Time: %d\n",
               processes[i].pid, processes[i].arrivalTime, processes[i].neededTime, processes[i].totalWaitTime);

        // 更新当前的时间
        currentTime += processes[i].neededTime;
    }

    // 计算等待平均时间
    double avgWaitTime = (double)totalWaitTime / NUM_PROCESSES;
    printf("Total Wait Time: %d\n", totalWaitTime);
    printf("Average Wait Time: %.2f\n", avgWaitTime);
}

int main() {
    struct PCB processes[NUM_PROCESSES];
    
    generateProcesses(processes);

    SJF(processes);

    return 0;
}
