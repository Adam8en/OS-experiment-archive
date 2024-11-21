// 2f.c
# include "q2.h"

Block *freeList = NULL;
PCBQueue *pcbQueue = NULL;
int global_block_id = 1; // 全局id计数器

/*创建一个内存块，确定内存块的：
    1.id 根据全局id计数器分配得到一个唯一id
    2.sizeK 由划分的父内存块大小右移一位得到
    3.startAddr 由父内存块的地址加上sizeK得到
    4.status 刚创建时默认为空闲，即true
    5.pid 初始时默认为空闲，即-1
    6.prev 指向前一块内存块
    7.next 指向后一块内存块
*/
Block* create_block(int sizeK, int startAddr, bool status, int pid){
    Block* new_block = (Block*)malloc(sizeof(Block));
    new_block->id = global_block_id++;
    new_block->sizeK = sizeK;
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
打印内存状态
*/
void print_memory_state(void){
    Block* current = freeList;
    while(current){
        if (current->status) {
            printf("free block id: %d, size: 2^%d, startAddr: %d\n",
                   current->id,current->sizeK,current->startAddr);
        }else {
            printf("used block id: %d, size: 2^%d, startAddr: %d, pid: %d\n",
                   current->id,current->sizeK,current->startAddr,current->pid);
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
找到一个空闲的内存块，要求内存块的大小大于给定的sizeK
*/
Block* find_free_block(int sizeK){
    Block* current = freeList;
    while(current){
        if (current->sizeK >= sizeK && current->status) return current;
        else current = current->next;
    }

    return NULL;
}

/*
给定一个内存块，找到其对应的伙伴内存块。
由于伙伴块的划分是通过对原内存块的2次幂划分得到，即存在新startAddr = 原startAddr + half_sizeK
故可由原startAddr + half_sizeK来定位其伙伴块。
由于伙伴系统的特性，这个地址可以由位运算得到
即 伙伴startAddr = 当前块startAddr ^ (1 << 当前块sizeK)
*/
Block* find_buddy(Block* block){
    int buddy_start = block->startAddr ^ (1 << block->sizeK);
    Block* current = freeList;
    while (current)
    {
        if (current->startAddr == buddy_start && current->sizeK == block->sizeK) return current;
        current = current->next;
    }

    return NULL;
}

/*
切分内存块，按照2的幂分配器对内存块进行划分
划分后的原内存块：
    1.id 原内存块的id不变
    2.sizeK 内存块大小需右移一位，即减半
    3.startAddr 父内存块的地址不变
    4.status 状态不变，由于被划分的内存块一定是空闲内存块所以一般为true
    5.pid pid不变，同上，一般为-1
    6.prev 不变
    7.next 指向被划分后的子内存块
划分后的新内存块：
    1.id 由原内存块的id+1得到
    2.sizeK 内存块大小右移一位，即减半
    3.startAddr 父内存块的地址+新的sizeK
    4.status 刚被创建属于空闲状态，故status为true
    5.pid 同上，一般为-1
    6.prev 指向被划分的原内存块
    7.next 只想原内存块的原next指向的对象
注意，如果原内存块的next对象存在，则也需要修改原next指向的对象的prev属性
使其指向新划分的内存块
*/
void split_block(Block* block){
    Block* new_block;
    int half_sizeK = block->sizeK - 1;
    new_block = create_block(half_sizeK,
                             block->startAddr + (1 << half_sizeK),
                             true, -1);
    new_block->prev = block;
    new_block->next = block->next;

    if (block->next){
        block->next->prev = new_block;
    }
    block->sizeK = half_sizeK;
    block->next = new_block;
    printf("Split block id: %d, size: 2^%d, startAddr: %d\n",
           block->id,block->sizeK,block->startAddr);
}

/*
为给定的进程分配内存，遵循伙伴系统的规则：
    1. 首先找到一块合适的内存块，要求内存块的内存大小比进程要求的内存大小大即可
    2. 然后不断地对内存块进行划分，直到内存块的大小刚好大于等于进程所要求的内存大小
        即若继续对内存进行一次划分，内存大小将不符合进程所要求的内存大小，刚好处于临界区
    3. 将该内存块分配给进程
        对于内存块： 更改其status和pid
        对于进程： 更改其status和blockID
*/
void allocate_memory(PCB* pcb){
    Block* block = find_free_block(pcb->neededMem);
    if (block){
        printf("Allocate free memory block for process #%d\n",pcb->pid);
        while(block->sizeK - 1 >= pcb->neededMem){
            split_block(block);
        }
        block->status = false;
        block->pid = pcb->pid;
        pcb->status = 1;
        pcb->blockID = block->id;
        print_memory_state();
    }else{
        printf("Cannot allocate a memory block for process %d\n",pcb->pid);
        return;
    }
}

/*
合并内存块，遵循伙伴系统的规则：
    1. 一个内存块被回收时，先找到其对应的伙伴块
    2. 然后探测其对应的伙伴情况是否空闲
    3. 如果伙伴空闲，则合并空闲伙伴向上递归
       如果伙伴不空闲，则停止合并，等待伙伴空闲
*/
void merge_blocks(void){
    Block* current = freeList;
    while(current){
        Block* buddy = find_buddy(current);
        // 确保伙伴存在、伙伴为空闲、当前块为空闲，方可进行合并
        if (!buddy || !buddy->status || !current->status){
            current = current->next;
            continue;
        }

        printf("Combine block id:%d and id:%d of size 2^%d\n",
               current->id,buddy->id,current->sizeK);

        // 如果找到的伙伴块是左伙伴块，则交换指针指向的对象，确保释放的一直是右伙伴块
        if (current->startAddr > buddy->startAddr){
            Block* temp = current;
            current = buddy;
            buddy = temp;
        }

        current->sizeK++;
        current->next = buddy->next;
        if (buddy->next) buddy->next->prev = current;
        free(buddy); // 释放伙伴块
        merge_blocks(); // 递归，尝试向上合并
        return; // 防止链表结构变化后指针失效，直接返回。
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
            printf("Recycle used memory block for process #%d of size 2^%d...\n",
                   pid,current->sizeK);
            merge_blocks();
            print_memory_state();
            return;
        }else current = current->next;
    }

    printf("Cannot find process %d\n",pid);
    return;
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
测试函数a：
    1. 初始化内存
    2. 创建进程
    3. 分配进程内存
    4. 释放进程内存
    5. 销毁内存和进程
假设n=3,即有3个进程，第1个进程申请的内存大小为2^7,第2个进程申请的内存大小为2^4，第3个进程申请的内存大小为2^8；
*/
void test_case_a(void){
    initialize_memory();
    pcbQueue = create_pcb_queue();

    // PCB *pcb1 = create_pcb(1, 7); // 2^7
    // PCB *pcb2 = create_pcb(2, 4); // 2^4
    // PCB *pcb3 = create_pcb(3, 8); // 2^8
    enqueue(pcbQueue,create_pcb(1, 7));
    enqueue(pcbQueue,create_pcb(2, 4));
    enqueue(pcbQueue,create_pcb(3, 8));
    print_pcb_queue();

    // allocate_memory(pcb1);
    // allocate_memory(pcb2);
    // allocate_memory(pcb3);

    while(!is_empty_queue(pcbQueue)){
        PCB* pcb = dequeue(pcbQueue);
        allocate_memory(pcb);
        free(pcb);
    }


    free_memory(1);
    free_memory(2);
    free_memory(3);

    delete_memory();
    delete_queue(pcbQueue);
}

/*
测试函数b：
假设有n=8个进程，每个进程所申请的内存块大小为2^k，其中k为随机整数，在[3,8]间产生。
*/
void test_case_b(void){
    initialize_memory();
    pcbQueue = create_pcb_queue();
    srand(time(NULL));

    for (int i = 1; i <= PROCESS_NUM; i++)
    {
        int neededMem = rand() % 6 + 3; // [3,8]
        enqueue(pcbQueue,create_pcb(i,neededMem));
    }
    print_pcb_queue();

    while(!is_empty_queue(pcbQueue)){
        PCB* pcb = dequeue(pcbQueue);
        allocate_memory(pcb);
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