#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    char* name; // 家庭成员姓名
    int op_amount; // 操作人员操作的钱款
    int op_times; // 剩余的操作次数，每人默认2次
    int is_withdraw; // 1代表为退款操作，0是储蓄操作
} Member;

Member members[] = {
    {"Dad", 0, 2},
    {"Mom", 0, 2},
    {"Grandma", 0, 2},
    {"Grandpa", 0, 2},
    {"Uncle", 0, 2},
    {"Mary", 0, 2},
    {"Sally", 0, 2},
};

int deposit_num[] = {10, 20, 30, 40, 50};
int withdraw_num[] = {50, 100};
int bank_account = 10;
pthread_mutex_t Lock;

void* Deposit(void* arg) {
    // 存款线程，传入参数为操作人员，在对银行账户进行操作时加锁
    Member* operator = (Member*)arg;

    pthread_mutex_lock(&Lock);
    bank_account += operator->op_amount;
    printf("%s deposited %d, now the bank account has %d\n",
           operator->name, operator->op_amount, bank_account);
    pthread_mutex_unlock(&Lock);

    //释放操作人员指向的姓名和本身的内存空间
    free(operator->name);
    free(operator);
    return NULL;
}

void* Withdraw(void* arg) {
    // 取款线程，传入参数为操作人员，在对银行账户进行操作时加锁
    Member* operator = (Member*)arg;

    pthread_mutex_lock(&Lock);
    bank_account -= operator->op_amount;
    printf("%s withdrew %d, now the bank account has %d\n",
           operator->name, operator->op_amount, bank_account);
    pthread_mutex_unlock(&Lock);

    //释放操作人员指向的姓名和本身的内存空间
    free(operator->name);
    free(operator);
    return NULL;
}

void load_operator(Member* rand_operator, int rand_guy) { 
    //处理向线程发送的数据，即填充操作人员的姓名、存取款状态、操作钱款
    rand_operator->name = strdup(members[rand_guy].name); // 填充操作人员的姓名
    if (rand_operator->name == NULL) {
        perror("strdup failed");
        exit(EXIT_FAILURE);
    }
    
    if (rand_guy <= 4) { // 根据随机到的人员编号判断该成员是存款还是取款
        rand_operator->is_withdraw = 0;
        rand_operator->op_amount = deposit_num[rand_guy]; // 操作钱款对应操作人员的编号
    } else {
        rand_operator->is_withdraw = 1;
        rand_operator->op_amount = withdraw_num[rand_guy - 5];
    }
}

void operate(void) {
    // 操作主体函数，处理随机存取逻辑并启动多线程
    int num = sizeof(members) / sizeof(members[0]); //成员数量
    int operate_num = 0; // 操作次数，即存取款次数之和，本体应为2*7=14
    int thread_index = 0; // 线程计数下标
    srand((unsigned int)time(NULL)); // 初始化随机数种子

    for (int i = 0; i < num; i++) {
        // 计算操作次数
        operate_num += members[i].op_times;
    }

    pthread_t* threads = malloc(operate_num * sizeof(pthread_t)); // 为操作总数动态分配相同个数的线程
    if (threads == NULL) {
        perror("malloc failed for threads");
        exit(EXIT_FAILURE);
    }

    while (operate_num > 0) {
        // 保证启动操作数量个线程
        // 取随机数生成随机的操作人员，如果该人员的操作次数已经用尽，就跳过该人员
        int rand_guy = rand() % num;
        if (members[rand_guy].op_times == 0) {
            continue;
        }
        members[rand_guy].op_times--;

        /*
        因为向线程传递的参数必须是一个指向特定地址的指针，线程启动后将向该地址读取数据
        所以必须用动态内存分配一个新的内存空间储存操作人员信息，并将该指针传递给线程
        如果用临时变量，操作人员的信息将只在一片相同的地址空间上进行迭代
        多线程读取数据时就会出现条件竞争
        故而此处必须使用动态内存分配，并在线程操作执行结束时由线程对分配的内存进行释放
        */
        Member* operator = malloc(sizeof(Member));
        if (operator == NULL) {
            perror("malloc failed for operator");
            exit(EXIT_FAILURE);
        }

        load_operator(operator, rand_guy); // 装载操作人员的信息

        if (operator->is_withdraw) { // 根据操作人员的信息决定调用存款还是取款线程
            pthread_create(&threads[thread_index++], NULL, Withdraw, operator);
        } else {
            pthread_create(&threads[thread_index++], NULL, Deposit, operator);
        }
        operate_num--;
    }

    for (int i = 0; i < thread_index; i++) { // 在多线程并发结束后等待多线程结束
        pthread_join(threads[i], NULL);
    }

    free(threads); // 操作结束，释放对线程分配的内存
}

int main() {
    pthread_mutex_init(&Lock, NULL); // 初始化互斥锁

    operate(); // 进行操作

    printf("Final bank account: %d\n", bank_account); // 回显最后银行账户的信息

    pthread_mutex_destroy(&Lock); // 销毁互斥锁

    return 0;
}
