# ifndef Q1_H
# define Q1_H

# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <time.h>
# define MEMORY_SIZE 1024
# define PROCESS_NUM 10

typedef struct Block {
    int id;               // 分区序号
    int size;            // 分区大小
    int startAddr;        // 分区起始地址
    bool status;          // true为空闲，false为占用
    int pid;              // 占用进程id, -1表示空闲
    struct Block *prev;   // 指向前一块内存分区
    struct Block *next;   // 指向后一块内存分区
} Block;

typedef struct PCB {
    int pid;              // 进程序号
    int neededMem;        // 需要的内存分区大小（2^neededMem）
    int status;           // 1：成功；-1：失败
    int blockID;          // 占用分区id，-1表示失败
    struct PCB *next;     // 指向下一个PCB
} PCB;

typedef struct PCBQueue {
    PCB* front;
    PCB* rear;
} PCBQueue;

Block* create_block(int size, int startAddr, bool status, int pid);
PCB* create_pcb(int pid, int neededMem);
PCBQueue* create_pcb_queue(void);
void initialize_memory(void);
void initialize_process(void);
void print_memory_state(void);
void print_pcb_queue(void);
void FF(PCB* pcb);
void NF(PCB* pcb);
void allocate_memory(PCB* pcb, Block* block);
void merge_memory(Block* block);
void free_memory(int pid);
int is_empty_queue(PCBQueue *Q);
void enqueue(PCBQueue* Q, PCB* pcb);
PCB* dequeue(PCBQueue* Q);
void delete_memory(void);
void delete_queue(PCBQueue* Q);
void test_case_a(void);
void test_case_b(void);

# endif