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

// 全局锁，用于有序分配法
pthread_mutex_t global_lock;

// 有序分配法：总是先拿编号小的叉子，再拿编号大的叉子
void pick_up_forks(int id) {
    int first_fork = id;
    int second_fork = (id + 1) % NUM_PHILOSOPHERS;
    
    // 确保总是先拿编号小的叉子
    if (first_fork > second_fork) {
        int temp = first_fork;
        first_fork = second_fork;
        second_fork = temp;
    }
    
    pthread_mutex_lock(&global_lock);  // 获取全局锁
    
    pthread_mutex_lock(&forks[first_fork]);
    printf("哲学家 %d 拿起了叉子 %d\n", id, first_fork);
    
    pthread_mutex_lock(&forks[second_fork]);
    printf("哲学家 %d 拿起了叉子 %d\n", id, second_fork);
    
    pthread_mutex_unlock(&global_lock);  // 释放全局锁
}

void put_down_forks(int id) {
    int first_fork = id;
    int second_fork = (id + 1) % NUM_PHILOSOPHERS;
    
    // 放下顺序不重要，但为了对称性，我们按拿起相反的顺序放下
    pthread_mutex_unlock(&forks[second_fork]);
    printf("哲学家 %d 放下了叉子 %d\n", id, second_fork);
    
    pthread_mutex_unlock(&forks[first_fork]);
    printf("哲学家 %d 放下了叉子 %d\n", id, first_fork);
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
    printf("哲学家就餐问题（使用有序分配法预防死锁）\n");
    printf("------------------------------------\n");
    
    // 初始化互斥锁（叉子）
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&forks[i], NULL);
    }
    
    // 初始化全局锁
    pthread_mutex_init(&global_lock, NULL);
    
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
    
    // 销毁全局锁
    pthread_mutex_destroy(&global_lock);
    
    return 0;
}

//编译：gcc 5_dinner_order_allocation.c -o 5_dinner_order_allocation -lpthread
//运行：./5_dinner_order_allocation