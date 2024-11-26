# include "q3.h"

int Sequence[MaxSequenceLen] = {0};
int PageAccessTime[MaxNumOfPages] = {0};
int PageArray[NumAllocatedPages] = {0};
int currentTime = 0;

/*
初始化操作
*/
void initialize(void){
    srand(time(NULL));
    generateSequence();
    for (int i = 0; i < NumAllocatedPages; i++){
        PageArray[i] = -1;
    }
}

/*
产生随机的访问页面序列，初始化Sequence
*/
void generateSequence(void){
    for (int i = 0; i < MaxSequenceLen; i++){
        Sequence[i] = rand() % MaxNumOfPages;
    }
}

/*
开始运行页面分配
*/
void allocatePages(void){
    int hit = 0;
    int isHit;

    printf("    SeqID        Working Set\n");
    for (int i = 0; i < MaxSequenceLen; i++){
        isHit = LRU(Sequence[i]);
        currentTime++;

        printf("%8d%12d%3d%3d",
               i + 1, PageArray[0], PageArray[1], PageArray[2]);

        if (isHit){
            printf("    *hit*\n");
            hit++;
        } else {
            printf("\n");
        }
    }
    print_result(hit);
}


/*
LRU算法：
    1. 首先按照顺序置换掉所有的-1值，因为-1代表空闲页面。
    2. 如果没有-1页面，代表所有的页面均已被分配。
       此时要添加新的页面，先检查该页面是否已经在工作集中：
        · 若在，则发生hit，说明OS无需从内存中调入新的页面，更新碰撞页面的使用时间，这将使其被置换的优先级降低
        · 若不在，意味着发生缺页错误。必须要置换出一个老页面，而置换顺序遵循LRU规则。
          即比较在Sequence[MaxSequenceLen]中页面在PageArray[NumAllocatedPages]中对应的值，
          访问时间越小的说明它过去被使用的时间越短，应该被优先置换
    3. 如果所有页面的使用时间相同，则按照FIFO的顺序对页面进行置换。
*/
int LRU(int pageID) {
    int isHit = 0;
    int replacePointer = -1;
    int minTime = currentTime;

    // 检查页面是否已经在工作集中
    for (int i = 0; i < NumAllocatedPages; i++) {
        if (PageArray[i] == pageID) {
            isHit = 1;  // 页面命中
            PageAccessTime[pageID] = currentTime;  // 更新访问时间
            break;
        }
    }

    if (isHit) {
        PageAccessTime[pageID] = currentTime;  // 更新访问时间
        return isHit;  // 如果是命中，直接返回
    }

    // 如果没有命中，发生缺页错误，找到最久未使用的页面进行置换
    for (int i = 0; i < NumAllocatedPages; i++) {
        if (PageArray[i] == -1) {
            PageArray[i] = pageID;  // 直接填充空闲页面
            PageAccessTime[pageID] = currentTime;
            return 0;
        }
    }

    // 如果所有页面已满，找到最久未使用的页面进行替换
    for (int i = 0; i < NumAllocatedPages; i++) {
        if (PageAccessTime[PageArray[i]] < minTime) {
            minTime = PageAccessTime[PageArray[i]];
            replacePointer = i;
        }
    }

    // 进行替换
    PageArray[replacePointer] = pageID;
    PageAccessTime[pageID] = currentTime;
    return 0;
}

/*
计算并打印缺页率
*/
void print_result(int hit){
    int miss = MaxSequenceLen - hit;
    double rate = (double)miss / (double)(hit + miss);

    printf("Hit = %d, Miss = %d\n",hit,miss);
    printf("Page fault Rate = %d/%d = %lf\n",miss,MaxSequenceLen,rate);
}