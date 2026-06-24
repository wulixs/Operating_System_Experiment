#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_READERS 5
#define NUM_WRITERS 3
#define READING_TIME 1
#define WRITING_TIME 2

pthread_mutex_t mutex;          // 保护共享变量的互斥锁
pthread_mutex_t rw_mutex;       // 读写锁
pthread_mutex_t write_priority; // 写优先锁
int read_count = 0;             // 当前读者数量
int write_waiting = 0;          // 等待写的写者数量

void* reader(void* arg) {
    int id = *((int*)arg);
    
    while (1) {
        // 尝试获取读权限
        pthread_mutex_lock(&write_priority);  // 检查是否有写者在等待
        pthread_mutex_lock(&mutex);           // 保护read_count
        
        read_count++;
        if (read_count == 1) {
            pthread_mutex_lock(&rw_mutex);    // 第一个读者锁住写者
        }
        
        pthread_mutex_unlock(&mutex);
        pthread_mutex_unlock(&write_priority);
        
        // 读操作
        printf("读者 %d 正在读取...             读者开始读取\n", id);
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
        pthread_mutex_lock(&write_priority);  // 写优先锁

        // ******
        printf("写者 %d 获取writer_priority成功\n", id);
        // ******

        write_waiting++;                     // 增加等待写者计数
        // pthread_mutex_unlock(&write_priority);   write_priority锁应在写者完成写操作后才释放，确保写者优先级
        
        pthread_mutex_lock(&rw_mutex);       // 获取读写锁
        
        // 写操作
        printf("写者 %d 正在写入...             写者开始写入\n", id);
        sleep(WRITING_TIME);
        printf("写者 %d 写入完成\n", id);
        
        // 释放写权限
        pthread_mutex_unlock(&rw_mutex);
        
        // pthread_mutex_lock(&write_priority);
        write_waiting--;                    // 减少等待写者计数
        // 写者完成写操作后才能释放写优先锁，确保写者优先级
        pthread_mutex_unlock(&write_priority);

        // ******
        printf("写者 %d 释放writer_priority\n", id);
        // ******
        
        // 模拟其他活动
        sleep(1);
    }
    
    return NULL;
}

int main() {

    // ******
    printf("姓名：董翔宇，学号：20243125\n");
    // ******

    printf("写优先的读者-写者问题\n");
    printf("--------------------\n");
    
    // 初始化互斥锁
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&rw_mutex, NULL);
    pthread_mutex_init(&write_priority, NULL);
    
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
    pthread_mutex_destroy(&write_priority);
    
    return 0;
}

//编译：gcc 7_rw_write_priority.c -o 7_rw_write_priority -lpthread

//运行：./7_rw_write_priority