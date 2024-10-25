#include "stdlib.h" //包含随机数产生函数
#include "stdio.h"  //标准输入输出函数库
#include "time.h"   //与时间有关的函数头文件
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

struct PCB
{
    char pid[64]; // 进程标识符，即进程的名字

    // 以下部分为用于进程调度的信息
    char state;        // ‘r’: 运行状态；‘w’:就绪状态
                       // ‘b’:阻塞状态
    int priority;      // 进程优先级
    int arrivalTime;   // 进程的创建时间(到达时间)
    int neededTime;    // 进程需要的运行时间
    int usedTime;      // 进程已累计运行的时间
    int totalWaitTime; // 进程已等待的CPU时间总和

    // 以下部分为进程的控制信息
    struct PCB *next; // 指向下一个PCB的链表指针
};

struct queue
{
    int priority;     // 该队列的优先级
    int timeSlice;    // 该队列的时间片长度
    struct PCB *list; // 指向该队列中进程PCB链表的头部
};

#define QUEUE_COUNT 5 // 设置队列长度为5
#define PROCESS_COUNT 20 // 设置进程总数为20
struct queue queues[QUEUE_COUNT];
int Time = 0; // 初始化全局时间为0，用于记录进程执行时消耗的时间
// sem_t semaphore_generator;
sem_t semaphore_scheduler;
pthread_mutex_t mutex;

void init(); // 初始化函数
void quit(); // 退出函数，销毁变量
void initQueues(); // 初始化队列
void *generateProcess(void *arg); // 产生进程的线程函数，生成器
void scheduler(); // 调度器
void executor(int queueIndex); // 执行器
void updatePCB(int sleepTime); // 更新PCB的数据，用于在睡眠后更新所有队列中进程的等待时间
int IsEmptyQueue(struct queue *q); // 检测队列是否为空
void EnQueue(struct queue *q, struct PCB *p); // 入队操作
struct PCB *DeQueue(struct queue *q); // 出队操作
void DeleteQueue(struct queue *q); // 删除队列操作
void DisplayQueue(); // 打印队列所有元素操作

int main(void)
{
    init();
    pthread_t generated_thread;
    pthread_create(&generated_thread, NULL, generateProcess, NULL); // 调用生成器线程函数

    scheduler(); // 执行调度器

    pthread_join(generated_thread, NULL); // 等待生成器函数线程完成

    quit();
    return 0;
}

void init()
{
    srand((unsigned)time(NULL));
    initQueues();

    // sem_init(&semaphore_generator, 0, 1);
    sem_init(&semaphore_scheduler, 0, 0);
    pthread_mutex_init(&mutex, NULL);
}

void quit()
{
    // sem_destroy(&semaphore_generator);
    sem_destroy(&semaphore_scheduler);
    pthread_mutex_destroy(&mutex);

    for (int i = 0; i < QUEUE_COUNT; i++)
    {
        DeleteQueue(&queues[i]);
    }
}

void initQueues()
{
    for (int i = 0; i < QUEUE_COUNT; i++)
    {
        queues[i].list = NULL;
        queues[i].priority = i + 1;
        queues[i].timeSlice = 10 * (1 << i); // 10,20,40,80,160ms
    }
}

void *generateProcess(void *arg)
{
    // 每次生成新进程时，都要用信号量通知scheduler同步处理新产生的进程
    // sem_wait(&semaphore_generator);
    for (int i = 0; i < PROCESS_COUNT; i++)
    {
        usleep((rand() % 100000) + 1000);
        struct PCB *newProcess = (struct PCB *)malloc(sizeof(struct PCB));
        snprintf(newProcess->pid, sizeof(newProcess->pid), "P%d", i); // 生成 PID
        newProcess->neededTime = (rand() % 199) + 2;
        newProcess->priority = 1;
        newProcess->next = NULL;
        newProcess->state = 'w';
        newProcess->totalWaitTime = 0;
        newProcess->arrivalTime = Time; // 当前的时间为抵达时间
        newProcess->usedTime = 0;

        // printf("the sem is comming!\n");

        pthread_mutex_lock(&mutex); 
        // printf("generator get the lock!\n");
        EnQueue(&queues[0], newProcess);
        // printf("generator get rid of the lock!\n");

        printf("Generator: Process %s is generated, neededTime = %d, arrivalTime = %d\n",
               newProcess->pid, newProcess->neededTime, newProcess->arrivalTime);
        pthread_mutex_unlock(&mutex);
        
        // printf("generator Post to scheduler\n");
        sem_post(&semaphore_scheduler);
        // sem_post(&semaphore_generator);
        // 一共会发送PROCESS_COUNT个信号量
    }
    return NULL;
}

void scheduler(){
    //scheduler的逻辑是执行多优先级队列，
    //如果收到generator发来的信号说明第一优先级队列出现新进程
    //所以需要立即转到第一队列进行处理。
    //用信号量控制scheduler是否重启执行逻辑，一个执行逻辑即从第一个队列开始依次往下调度
    //假设scheduler一直收不到generator的信号量，scheduler最终将会把所有在优先级队列中的进程释放
    /*
    a:首先检查第一个队列是否为空：
        1.若非空，则调用执行器执行队首进程。并依情况判断是否交到下一个优先级队列，回到a
        2.若为空，则开始调度下一个队列
    */

   // 这个变量统计已经接受到的进程信号量，当processCount=PROCESSCOUNT时，说明进程已经全部执行完毕，调度器退出。
   int processCount=0;

   sem_wait(&semaphore_scheduler);
   /*
   这里processCount递增有两种情况：
    1.scheduler第一次被调用时会递增一次计数
    2.假如队列内全部进程执行完毕，但是generator还在工作（处于睡眠状态没来得及产生新的进程），scheduler会在这里等待信号量。
   */
   processCount++;

   while(processCount<PROCESS_COUNT){
    // 从此处开始，由第一个优先级队列开始调度。
    // 所以下面的进程处理逻辑在检测到信号量时，应该退出for循环。
    begin:
    for (int i=0;i<QUEUE_COUNT;i++){
        // 一个进程处理逻辑，若没有信号量打断将会处理所有进程
        // 所以需要建立scheduler退出机制，保证接受到信号量后退出循环。
        // 依次处理所有队列，每调用一次executor对应CPU执行了对应时间片的任务
        // 在时间片结束后：1.对该进程的情况进行调度 2.检查信号量，是否有新的进程抵达第一级优先队列
        while (!IsEmptyQueue(&queues[i]))
            {
                //while循环将重复处理当前队列直到队列排空为止，处理过程中需要留意是否接受到generator信号量
                if (sem_trywait(&semaphore_scheduler) == 0) {
                    // 如果信号量被设置，立即退出循环，回到进程处理逻辑外部从第一个队列重新开始。
                    // 这里processCount递增代表着又有一个新的进程加入到了队列中
                    // 有一个问题：如果generator短时间内插入了多个进程，会导致scheduler <空转一次>，但是不影响计数正常使用 
                    processCount++;
                    goto begin;
                }
                pthread_mutex_lock(&mutex);
                // 当前队列不为空，调用executor
                // 传递队列编号给executor，executor将把进程取出/出队进行处理，由scheduler决定进程是继续入队还是释放
                struct PCB *tmp = queues[i].list;

                pthread_mutex_unlock(&mutex);

                executor(i);

                pthread_mutex_lock(&mutex);

                // 判断该进程如何调度
                if (tmp->usedTime < tmp->neededTime)
                {
                    // 如果执行进程还未结束，则将之下放一级队列
                    if (tmp->priority < QUEUE_COUNT)
                    {
                        // 如果进程还可以继续往下放置
                        //  pthread_mutex_lock(&mutex);
                        tmp->priority++;
                        EnQueue(&queues[i + 1], tmp);
                        printf("Scheduler: Process %s is moved to queue %d, priority = %d\n",
                               tmp->pid, i + 2, tmp->priority);
                        DisplayQueue(); // 加锁移动至函数内部
                    }
                    else
                    {
                        // 如果队列已经无法下移，为了确保系统正确运行，则直接丢弃/释放进程。
                        printf("Scheduler: Process %s is running overtime, total waiting time = %d, aborted.\n",
                               tmp->pid, tmp->totalWaitTime);
                        free(tmp);
                        DisplayQueue();
                    }
                }
                else
                {
                    // 进程执行完毕，释放进程
                    printf("Scheduler: Process %s finished, total waiting time = %d\n",
                           tmp->pid, tmp->totalWaitTime);
                    DisplayQueue();
                    free(tmp);
                }
                pthread_mutex_unlock(&mutex);
            }
            // 对队伍进程进行调度到这里为止

    // 这里是执行逻辑的末尾，说明整个多优先级任务队列中已经没有等待的任务
    // scheduler应该进入休眠状态等待generator的信号量，或者结束调度退出程序
    }
   }
}

void executor(int queueIndex)
{
    pthread_mutex_lock(&mutex);
    // printf("executor get the lock!\n");
    struct PCB *tmp = DeQueue(&queues[queueIndex]);
    // printf("executor get the %s!\n",tmp->pid);

    int timeSlice = queues[queueIndex].timeSlice;
    int sleepTime = (timeSlice+tmp->usedTime > tmp->neededTime) ? tmp->neededTime-tmp->usedTime : timeSlice;

    usleep(sleepTime * 1000);
    Time += sleepTime;
    // printf("executor is trying to update all PCB.\n");
    updatePCB(sleepTime);

    tmp->usedTime += sleepTime;
    // printf("executor is going to release lock.\n");
    pthread_mutex_unlock(&mutex);
    // printf("executor release lock!\n");

    printf("Executor: Process %s in queue %d consumes %d ms\n",
           tmp->pid, queueIndex + 1, sleepTime);
}

void updatePCB(int sleepTime)
{
    // 更新队列中所有进程的等待时间
    struct PCB *tmp;
    struct PCB *tempQueue[PROCESS_COUNT]; // 临时队列数组
    int count = 0; // 临时队列计数

    for (int i = 0; i < QUEUE_COUNT; i++)
    {
        while (!IsEmptyQueue(&queues[i]))
        {
            // 这里对队列进行操作，但是父函数executor已经锁住临界区，所以可以不用加锁。
            // printf("queue%d is not empty,update it!\n",i+1);
            tmp = DeQueue(&queues[i]);
            // printf("trying to update %s\n",tmp->pid);

            tmp->totalWaitTime += sleepTime; 

            // 将更新后的进程存入临时队列
            tempQueue[count++] = tmp;
        }

        for (int j = 0; j < count; j++)
        {
            EnQueue(&queues[i], tempQueue[j]);
        }
        count = 0; // 重置计数器
    }
    // printf("PCB finished updating!\n");
}


int IsEmptyQueue(struct queue *q)
{
    return (q->list == NULL);
}

void EnQueue(struct queue *q, struct PCB *p)
{
    if (q->list == NULL)
    {
        q->list = p;
    }
    else
    {
        struct PCB *tmp;
        tmp = q->list;
        while (tmp->next != NULL)
        {
            tmp = tmp->next;
        }
        tmp->next = p;
    }
}

struct PCB *DeQueue(struct queue *q)
{
    // printf("trying to dequeue queue%d\n",q->priority);
    if (IsEmptyQueue(q))
    {
        // printf("The Queue is empty!\n");
        return NULL;
    }
    else
    {
        struct PCB *p;
        p = q->list;
        q->list = q->list->next;
        p->next=NULL; //切断出队元素与原队列之间的黏连
        return p;
    }
}

void DeleteQueue(struct queue *q)
{
    struct PCB *p;
    while (q->list)
    {
        p = q->list;
        q->list = q->list->next;
        free(p);
    }
}

void DisplayQueue()
{
    // pthread_mutex_lock(&mutex);
    struct PCB *tmp;
    struct PCB *tempQueue[PROCESS_COUNT]; // 临时队列数组
    int count;

    for (int i = 0; i < QUEUE_COUNT; i++)
    {
        printf("Queue %d: ", i + 1);
        count = 0; // 重置计数器
        
        while (!IsEmptyQueue(&queues[i]))
        {
            tmp = DeQueue(&queues[i]);
            if (tmp == NULL)
                break; // 安全处理

            printf("%s ", tmp->pid); // 打印进程 PID
            tempQueue[count++] = tmp; // 存储进程到临时队列
        }
        printf("\n");

        // 将临时队列中的进程重新入队
        for (int j = 0; j < count; j++)
        {
            EnQueue(&queues[i], tempQueue[j]);
        }
    }
    // pthread_mutex_unlock(&mutex);
}
