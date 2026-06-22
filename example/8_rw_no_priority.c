#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_READERS 5
#define NUM_WRITERS 3
#define READING_TIME 1
#define WRITING_TIME 2

pthread_mutex_t mutex;          // 保护read_count的互斥锁
pthread_mutex_t rw_mutex;       // 读写锁
int read_count = 0;             // 当前读者数量

void* reader(void* arg) {
    int id = *((int*)arg);
    
    while (1) {
        // 获取读权限
        pthread_mutex_lock(&mutex);
        read_count++;
        if (read_count == 1) {
            pthread_mutex_lock(&rw_mutex);  // 第一个读者锁住写者
        }
        pthread_mutex_unlock(&mutex);
        
        // 读操作
        printf("读者 %d 正在读取...\n", id);
        sleep(READING_TIME);
        printf("读者 %d 读取完成\n", id);
        
        // 释放读权限
        pthread_mutex_lock(&mutex);
        read_count--;
        if (read_count == 0) {
            pthread_mutex_unlock(&rw_mutex);  // 最后一个读者释放写者
        }
        pthread_mutex_unlock(&mutex);
        
        // 模拟其他活动
        sleep(1);
    }
    
    return NULL;
}

void* writer(void* arg) {
    int id = *((int*)arg);
    
    while (1) {
        // 获取写权限
        pthread_mutex_lock(&rw_mutex);
        
        // 写操作
        printf("写者 %d 正在写入...\n", id);
        sleep(WRITING_TIME);
        printf("写者 %d 写入完成\n", id);
        
        // 释放写权限
        pthread_mutex_unlock(&rw_mutex);
        
        // 模拟其他活动
        sleep(1);
    }
    
    return NULL;
}

int main() {
    printf("无优先级的读者-写者问题\n");
    printf("----------------------\n");
    
    // 初始化互斥锁
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&rw_mutex, NULL);
    
    pthread_t readers[NUM_READERS], writers[NUM_WRITERS];
    int reader_ids[NUM_READERS], writer_ids[NUM_WRITERS];
    
    // 创建读者线程
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }
    
    // 创建写者线程
    for (int i = 0; i < NUM_WRITERS; i++) {
        writer_ids[i] = i;
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }
    
    // 等待线程结束（实际上会无限运行）
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }
    
    // 销毁互斥锁
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&rw_mutex);
    
    return 0;
}

//编译：gcc 8_rw_no_priority.c -o 8_rw_no_priority -lpthread
//运行：./8_rw_no_priority