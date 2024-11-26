# ifndef Q3_H
# define Q3_H

# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# define MaxSequenceLen 20 // 随机生成的访问序列长度
# define MaxNumOfPages 10 // 进程所需要的最大页面数
# define NumAllocatedPages 3 // OS给该进程分配的页面空间大小

void initialize(void);
void generateSequence(void);
void allocatePages(void);
int LRU(int pageID);
void print_result(int hit);

# endif