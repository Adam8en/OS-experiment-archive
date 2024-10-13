#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    char* name; 
    int op_amount; 
    int op_times; 
    int is_withdraw; 
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
pthread_cond_t Cond;

void* Deposit(void* arg) {
    // 新增特性：存款时将尝试唤醒所有条件变量，以恢复阻塞的取款线程
    Member* operator = (Member*)arg;

    pthread_mutex_lock(&Lock);
    bank_account += operator->op_amount;
    printf("%s deposited %d, now the bank account has %d\n",
           operator->name, operator->op_amount, bank_account);
    pthread_cond_broadcast(&Cond); // 存款后，唤醒所有条件变量，再次尝试取款
    pthread_mutex_unlock(&Lock);

    free(operator->name);
    free(operator);
    return NULL;
}

void* Withdraw(void* arg) {
    // 新增特性：若取款后余额将为负数，则阻塞线程并释放锁
    Member* operator = (Member*)arg;

    pthread_mutex_lock(&Lock);

    while(bank_account<operator->op_amount){
        // 若此时银行余额小于取款操作钱款，则阻塞线程，释放锁。
        printf("%s is waiting to withdraw %d, but only %d is available, operation blockage.\n",
        operator->name,operator->op_amount,bank_account);
        pthread_cond_wait(&Cond,&Lock);
    }

    bank_account -= operator->op_amount;
    printf("%s withdrew %d, now the bank account has %d\n",
           operator->name, operator->op_amount, bank_account);

    pthread_mutex_unlock(&Lock);

    free(operator->name);
    free(operator);
    return NULL;
}

void load_operator(Member* rand_operator, int rand_guy) { 
    rand_operator->name = strdup(members[rand_guy].name); 
    if (rand_operator->name == NULL) {
        perror("strdup failed");
        exit(EXIT_FAILURE);
    }
    
    if (rand_guy <= 4) { 
        rand_operator->is_withdraw = 0;
        rand_operator->op_amount = deposit_num[rand_guy];
    } else {
        rand_operator->is_withdraw = 1;
        rand_operator->op_amount = withdraw_num[rand_guy - 5];
    }
}

void operate(void) {
    int num = sizeof(members) / sizeof(members[0]); 
    int operate_num = 0; 
    int thread_index = 0; 
    srand((unsigned int)time(NULL));

    for (int i = 0; i < num; i++) {
        operate_num += members[i].op_times;
    }

    pthread_t* threads = malloc(operate_num * sizeof(pthread_t)); 
    if (threads == NULL) {
        perror("malloc failed for threads");
        exit(EXIT_FAILURE);
    }

    while (operate_num > 0) {
        int rand_guy = rand() % num;
        if (members[rand_guy].op_times == 0) {
            continue;
        }
        members[rand_guy].op_times--;

        Member* operator = malloc(sizeof(Member));
        if (operator == NULL) {
            perror("malloc failed for operator");
            exit(EXIT_FAILURE);
        }

        load_operator(operator, rand_guy); 

        if (operator->is_withdraw) { 
            pthread_create(&threads[thread_index++], NULL, Withdraw, operator);
        } else {
            pthread_create(&threads[thread_index++], NULL, Deposit, operator);
        }
        operate_num--;
    }

    for (int i = 0; i < thread_index; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads); 
}

int main() {
    pthread_mutex_init(&Lock, NULL); 
    pthread_cond_init(&Cond,NULL); // 初始化条件变量

    operate(); 

    printf("Final bank account: %d\n", bank_account); 

    pthread_mutex_destroy(&Lock); 
    pthread_cond_destroy(&Cond); // 销毁条件变量

    return 0;
}
