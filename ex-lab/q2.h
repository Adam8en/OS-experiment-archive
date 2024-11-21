// 2.h
# ifndef Q2_H
# define Q2_H

# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <time.h>
# define MEMORY_SIZE 10
# define PROCESS_NUM 8

typedef struct Block {
    int id;               // 分区序号
    int sizeK;            // 分区大小，以2^sizeK表示
    int startAddr;        // 分区起始地址
    bool status;          // true为空闲，false为占用
    int pid;              // 占用进程id, -1表示空闲
    struct Block *prev;   // 指向前一块内存分区
    struct Block *next;   // 指向后一块内存分区
} Block;

// 定义进程PCB结构体
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

Block* create_block(int sizeK, int startAddr, bool status, int pid);
PCB* create_pcb(int pid, int neededMem);
PCBQueue* create_pcb_queue(void);
void initialize_memory(void);
void print_memory_state(void);
void print_pcb_queue(void);
Block* find_free_block(int sizeK);
Block* find_buddy(Block* block);
void split_block(Block* block);
void allocate_memory(PCB* pcb);
void merge_blocks(void);
void free_memory(int pid);
int is_empty_queue(PCBQueue *Q);
void enqueue(PCBQueue* Q, PCB* pcb);
PCB* dequeue(PCBQueue* Q);
void test_case_a(void);
void test_case_b(void);
void delete_memory(void);
void delete_queue(PCBQueue* Q);

# endif