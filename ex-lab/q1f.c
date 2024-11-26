# include "q1.h"

Block *freeList = NULL;
PCBQueue *pcbQueue = NULL;
int global_block_id = 1; // 全局id计数器

/*创建一个内存块，确定内存块的：
    1.id 根据全局id计数器分配得到一个唯一id
    2.size 内存块的大小
    3.startAddr 由父内存块的地址加上size得到
    4.status 刚创建时默认为空闲，即true
    5.pid 初始时默认为空闲，即-1
    6.prev 指向前一块内存块
    7.next 指向后一块内存块
*/
Block* create_block(int size, int startAddr, bool status, int pid){
    Block* new_block = (Block*)malloc(sizeof(Block));
    new_block->id = global_block_id++;
    new_block->size = size;
    new_block->startAddr = startAddr;
    new_block->status = status;
    new_block->pid = pid;
    new_block->prev = NULL;
    new_block->next = NULL;

    return new_block;
}

/*
创建一个进程，确定进程的：
    1.pid 进程的序号
    2.neededMem 进程所需要的内存空间大小
    3.status 进程的状态，刚创建时默认未分配内存，即-1
    4.blockID 表示进程占用的内存块序号，初始化为未占用，即-1
    5.next 指向下一个进程，初始化为NULL
*/
PCB* create_pcb(int pid, int neededMem){
    PCB* new_pcb = (PCB*)malloc(sizeof(PCB));
    new_pcb->pid = pid;
    new_pcb->neededMem = neededMem;
    new_pcb->status = -1;
    new_pcb->blockID = -1;
    new_pcb->next = NULL;

    return new_pcb;
}

/*
创建pcb队列
*/
PCBQueue* create_pcb_queue(void){
    PCBQueue* Q = (PCBQueue*)malloc(sizeof(PCBQueue));

    Q->front = Q->rear =NULL;
    return Q;
}

/*
初始化内存块为一块大内存，容量为1024kb
*/
void initialize_memory(void){
    freeList = create_block(MEMORY_SIZE, 0, true, -1);
    freeList->prev = freeList->next = NULL;
}

/*
初始化进程，创建PROCESS_NUM个进程并加入PCB进程队列
*/
void initialize_process(void){
    PCB* temp;
    int random_memory;
    for (int i = 1; i <= PROCESS_NUM; i++){
        random_memory = rand() % 101 + 100;
        temp = create_pcb(i,random_memory);
        enqueue(pcbQueue,temp);
    }
}

/*
打印内存状态
*/
void print_memory_state(void){
    Block* current = freeList;
    while(current){
        if (current->status) {
            printf("free block id: %d, size: %d, startAddr: %d\n",
                   current->id,current->size,current->startAddr);
        }else {
            printf("used block id: %d, size: %d, startAddr: %d, pid: %d\n",
                   current->id,current->size,current->startAddr,current->pid);
        }
        current = current->next;
    }
    printf("\n");
}

/*
打印进程状态
*/
void print_pcb_queue(void){
    PCB* current = pcbQueue->front;

    printf("PCB Queue State:\n");
    while (current){
        printf("#%d neededMem:%d\n",current->pid,current->neededMem);
        current = current->next;
    }
    printf("\n");
}

/*
用首次适应算法为给定的进程分配内存：
    1. 从空闲分区链表的第一项开始往后遍历，找到一个足够大的空闲分区
    2. 如果没找到，则提示内存分配失败，接着继续为下一个进程分配内存
    3. 如果找到了，则给该进程分配，并更新空闲分区链表
    4. 当对一块空闲分区进行划分时，
       在这块空闲分区的地址范围内随机产生一个划分的开始位置，
       然后划分出当前进程大小的分区
划分空闲分区会产生三个或两个块（当划分开始位置是空闲块起始位置或者终止位置与空闲块终止位置重合时）。
*/
void FF(PCB* pcb){
    Block* current = freeList;

    while (current)
    {
        if (current->status && current->size >= pcb->neededMem){
            allocate_memory(pcb,current);
            return;
        }
        current = current->next;
    }
    printf("Cannot allocate memory for process #%d\n",pcb->pid);
}

/*
用循环首次适应算法为给定的进程分配内存
区别在于，每次不是从头开始检索空闲内存块，而是从上一次分配的地方开始检索
*/
void NF(PCB* pcb){
    static Block* lastAllocated = NULL;
    Block* current = lastAllocated ? lastAllocated->next : freeList;

    if(!current) current = freeList;

    for (int i = 0; i<PROCESS_NUM; i++){
        if (current->status && current->size >= pcb->neededMem){
            allocate_memory(pcb,current);
            lastAllocated = current;
            return;
        }
        current = current->next;
        if(!current) current = freeList;
    }
    printf("Cannot allocate memory for process #%d\n",pcb->pid);
}

/*
给定进程块和要分配的blockID，将在该空闲内存块上划分内存分配给进程；
并且更新内存块和PCB的信息
*/
void allocate_memory(PCB* pcb, Block* block){
    Block* leftBlock = NULL; // 指向被划分内存块的左侧
    Block* middleBlock = NULL; // 指向被划分的内存块
    Block* rightBlock = NULL; // 指向被划分内存块的右侧
    int startAddr = block->startAddr + rand() % (block->size - pcb->neededMem + 1); // 随机选择的起始地址

    pcb->blockID = block->id;
    pcb->status = 1;

    printf("Allocate free memory block for process #%d\n",pcb->pid);

    // 从空闲块头部开始划分
    if (startAddr == block->startAddr){
        rightBlock = create_block(block->size - pcb->neededMem, startAddr,
                                  true,-1);

        middleBlock = block;
        middleBlock->pid = pcb->pid;
        middleBlock->status = false;
        middleBlock->size = pcb->neededMem;
    } // 从空闲块尾部开始划分
    else if (startAddr + pcb->neededMem == block->startAddr + block->size){
        leftBlock = block;
        middleBlock = create_block(pcb->neededMem, startAddr,false,pcb->pid);

        leftBlock->size = block->size - pcb->neededMem;
    } // 从空闲块中部开始划分
    else {
        leftBlock = block;
        middleBlock = create_block(pcb->neededMem,startAddr,false,pcb->pid);
        rightBlock = create_block(block->startAddr + block->size - startAddr - pcb->neededMem,
                                  startAddr + pcb->neededMem,true,-1);

        leftBlock->size = startAddr - block->startAddr;
    }

    if (!leftBlock && rightBlock){
        rightBlock->prev = middleBlock;
        rightBlock->next = middleBlock->next;
        middleBlock->next = rightBlock;
    }else if (leftBlock && !rightBlock){
        middleBlock->prev = leftBlock;
        middleBlock->next = leftBlock->next;

        if (leftBlock->next) leftBlock->next->prev = rightBlock;
        leftBlock->next = middleBlock;
    }else if (leftBlock && rightBlock){
        middleBlock->prev = leftBlock;
        middleBlock->next = rightBlock;
        rightBlock->prev = middleBlock;
        rightBlock->next = leftBlock->next;

        if (leftBlock->next) leftBlock->next->prev = rightBlock;
        leftBlock->next = middleBlock;
    }

    print_memory_state();
}

/*
合并空闲的内存:
    1. 检查其左右内存块是否可以用于合并
    2. 对于空闲的左内存块，更改其sizeK
    3. 对于空闲的右内存块，更改其startAddr与sizeK
    4. 更改指针关系
    5. 释放内存
*/
void merge_memory(Block* block) {
    Block* leftBlock = block->prev;
    Block* rightBlock = block->next;

    // 合并右侧空闲块
    if (rightBlock && rightBlock->status) {
        printf("Combine block id:%d (size: %d) with id:%d (size: %d)\n",
               block->id, block->size, rightBlock->id, rightBlock->size);

        block->size += rightBlock->size;
        block->next = rightBlock->next;

        if (rightBlock->next) {
            rightBlock->next->prev = block;
        }

        free(rightBlock);
    }

    // 合并左侧空闲块
    if (leftBlock && leftBlock->status) {
        printf("Combine block id:%d (size: %d) with id:%d (size: %d)\n",
               leftBlock->id, leftBlock->size, block->id, block->size);

        leftBlock->size += block->size;
        leftBlock->next = block->next;

        if (block->next) {
            block->next->prev = leftBlock;
        }

        free(block);
    }
}


/*
给定进程的pid，释放该进程及分配给它的内存块。
    对于进程：不变
    对于内存块：更改status、pid属性，并调用合并内存块
*/
void free_memory(int pid){
    Block* current = freeList;

    while (current)
    {
        if (current->pid == pid){
            current->status = true;
            current->pid = -1;
            printf("Recycle used memory block for process #%d of size %d...\n",
                   pid,current->size);
            merge_memory(current);
            print_memory_state();
            return;
        }
        current = current->next;
    }
    printf("Cannot free memory for process #%d\n",pid);
}

/*
判断当前进程队列是否为空
*/
int is_empty_queue(PCBQueue *Q){
    return Q->front==NULL;
}

/*
将进程加入进程队列
*/
void enqueue(PCBQueue* Q, PCB* pcb){
    if (Q->rear) Q->rear->next = pcb;

    Q->rear = pcb;

    if (Q->front == NULL) Q->front = Q->rear;
}

/*
将进程从队列中出队
*/
PCB* dequeue(PCBQueue* Q){
    if (is_empty_queue(Q)){
        printf("PCB Queue is Empty!\n");
        return NULL;
    }else{
        PCB* temp = Q->front;
        Q->front = Q->front->next;
        return temp;
    }
}

/*
测试函数，用于测试FF算法。
*/
void test_case_a(void){
    pcbQueue = create_pcb_queue();
    initialize_memory();
    initialize_process();
    srand(time(NULL));

    print_pcb_queue();

    while(!is_empty_queue(pcbQueue)){
        PCB* pcb = dequeue(pcbQueue);
        FF(pcb);
        free(pcb);
    }

    for (int i = 1; i <= PROCESS_NUM; i++)
    {
        free_memory(i);
    }

    delete_memory();
    delete_queue(pcbQueue);
}

/*
测试函数，用于测试NF算法。
*/
void test_case_b(void){
    pcbQueue = create_pcb_queue();
    initialize_memory();
    initialize_process();
    srand(time(NULL));

    print_pcb_queue();

    while(!is_empty_queue(pcbQueue)){
        PCB* pcb = dequeue(pcbQueue);
        NF(pcb);
        free(pcb);
    }

    for (int i = 1; i <= PROCESS_NUM; i++)
    {
        free_memory(i);
    }

    delete_memory();
    delete_queue(pcbQueue);
}

/*
删除内存分配的空间
*/
void delete_memory(void){
    Block *temp = freeList;
    freeList->next = NULL;
    free(temp);
}

/*
删除PCB进程队列
*/
void delete_queue(PCBQueue* Q){
    PCB* temp;
    while (Q->front){
        temp = Q->front;
        Q->front = Q->front->next;
        free(temp);
    }
    free(Q);
}