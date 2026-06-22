#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_PHILOSOPHERS 5
#define EATING_TIME 2
#define THINKING_TIME 1

pthread_mutex_t forks[NUM_PHILOSOPHERS];
pthread_t philosophers[NUM_PHILOSOPHERS];
int philosopher_ids[NUM_PHILOSOPHERS];

// 预先分配顺序：偶数编号哲学家先拿左边叉子，奇数编号先拿右边
void pick_up_forks(int id) {
    int left_fork = id;
    int right_fork = (id + 1) % NUM_PHILOSOPHERS;
    
    if (id % 2 == 0) {
        // 偶数编号哲学家：先左后右
        pthread_mutex_lock(&forks[left_fork]);
        printf("哲学家 %d 拿起了左边的叉子 %d\n", id, left_fork);
        pthread_mutex_lock(&forks[right_fork]);
        printf("哲学家 %d 拿起了右边的叉子 %d\n", id, right_fork);
    } else {
        // 奇数编号哲学家：先右后左
        pthread_mutex_lock(&forks[right_fork]);
        printf("哲学家 %d 拿起了右边的叉子 %d\n", id, right_fork);
        pthread_mutex_lock(&forks[left_fork]);
        printf("哲学家 %d 拿起了左边的叉子 %d\n", id, left_fork);
    }
}

void put_down_forks(int id) {
    int left_fork = id;
    int right_fork = (id + 1) % NUM_PHILOSOPHERS;
    
    pthread_mutex_unlock(&forks[left_fork]);
    printf("哲学家 %d 放下了左边的叉子 %d\n", id, left_fork);
    pthread_mutex_unlock(&forks[right_fork]);
    printf("哲学家 %d 放下了右边的叉子 %d\n", id, right_fork);
}

void* philosopher(void* arg) {
    int id = *((int*)arg);
    
    while (1) {
        // 思考
        printf("哲学家 %d 正在思考...\n", id);
        sleep(THINKING_TIME);
        
        // 拿叉子
        printf("哲学家 %d 饿了，准备拿叉子\n", id);
        pick_up_forks(id);
        
        // 吃饭
        printf("哲学家 %d 开始吃饭\n", id);
        sleep(EATING_TIME);
        
        // 放回叉子
        printf("哲学家 %d 吃完啦\n", id);
        put_down_forks(id);
    }
    
    return NULL;
}

int main() {
    printf("哲学家就餐问题（使用预先分配法预防死锁）\n");
    printf("------------------------------------\n");
    
    // 初始化互斥锁（叉子）
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&forks[i], NULL);
    }
    
    // 创建哲学家线程
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        philosopher_ids[i] = i;
        pthread_create(&philosophers[i], NULL, philosopher, &philosopher_ids[i]);
    }
    
    // 等待所有线程完成（实际上会无限运行）
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }
    
    // 销毁互斥锁
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&forks[i]);
    }
    
    return 0;
}

//编译：gcc 4_dinner_pre_allocation.c -o 4_dinner_pre_allocation -lpthread
//运行：./4_dinner_pre_allocation