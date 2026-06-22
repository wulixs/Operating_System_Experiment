#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_JOBS 100
#define MAX_FILENAME 256
#define BUFFER_SIZE 1024

// 打印任务结构体
typedef struct {
    char filename[MAX_FILENAME];
    int job_id;
    int pages;
} PrintJob;

// 打印队列结构体
typedef struct {
    PrintJob jobs[MAX_JOBS];
    int front;
    int rear;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} PrintQueue;

PrintQueue print_queue;  // 全局打印队列
int next_job_id = 1;    // 下一个任务ID

// 初始化打印队列
void init_queue(PrintQueue *queue) {
    queue->front = 0;
    queue->rear = -1;
    queue->count = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);
}

// 添加打印任务到队列
void enqueue(PrintQueue *queue, PrintJob job) {
    pthread_mutex_lock(&queue->mutex);
    
    // 等待队列有空位
    while (queue->count >= MAX_JOBS) {
        printf("打印队列已满，等待中...\n");
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }
    
    // 添加到队列
    queue->rear = (queue->rear + 1) % MAX_JOBS;
    queue->jobs[queue->rear] = job;
    queue->count++;
    
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);
}

// 从队列获取打印任务
PrintJob dequeue(PrintQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    
    // 等待队列有任务
    while (queue->count <= 0) {
        printf("打印队列为空，等待中...\n");
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    
    // 从队列取出
    PrintJob job = queue->jobs[queue->front];
    queue->front = (queue->front + 1) % MAX_JOBS;
    queue->count--;
    
    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);
    
    return job;
}

// 模拟打印任务
void* printer_thread(void* arg) {
    while (1) {
        PrintJob job = dequeue(&print_queue);
        
        printf("[打印机] 开始打印任务 #%d: %s (%d页)\n", 
               job.job_id, job.filename, job.pages);
        
        // 模拟打印时间 (1秒/页)
        for (int i = 1; i <= job.pages; i++) {
            printf("[打印机] 正在打印任务 #%d: 第%d页/%d\n", 
                   job.job_id, i, job.pages);
            sleep(1);
        }
        
        printf("[打印机] 完成打印任务 #%d\n", job.job_id);
    }
    return NULL;
}

// 生成测试打印任务
void generate_test_jobs(int num_jobs) {
    const char* test_files[] = {
        "document1.pdf", "report.doc", "presentation.pptx",
        "image.jpg", "data.csv", "notes.txt"
    };
    const int num_files = sizeof(test_files) / sizeof(test_files[0]);
    
    for (int i = 0; i < num_jobs; i++) {
        PrintJob job;
        strncpy(job.filename, test_files[rand() % num_files], MAX_FILENAME);
        job.job_id = next_job_id++;
        job.pages = (rand() % 10) + 1;  // 1-10页
        
        enqueue(&print_queue, job);
        printf("[生成器] 已添加打印任务 #%d: %s (%d页)\n", 
               job.job_id, job.filename, job.pages);
        
        // 随机延迟 (0-2秒)
        usleep((rand() % 2000000));
    }
}

// 用户交互添加打印任务
void* user_input_thread(void* arg) {
    char input[MAX_FILENAME + 10];
    
    while (1) {
        printf("\n请输入要打印的文件名和页数 (格式: 文件名 页数): ");
        fgets(input, sizeof(input), stdin);
        
        char filename[MAX_FILENAME];
        int pages;
        
        if (sscanf(input, "%255s %d", filename, &pages) == 2) {
            PrintJob job;
            strncpy(job.filename, filename, MAX_FILENAME);
            job.job_id = next_job_id++;
            job.pages = pages;
            
            enqueue(&print_queue, job);
            printf("[用户] 已添加打印任务 #%d: %s (%d页)\n", 
                   job.job_id, job.filename, job.pages);
        } else {
            printf("输入格式错误，请使用: 文件名 页数\n");
        }
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    init_queue(&print_queue);
    
    // 创建打印机线程
    pthread_t printer;
    pthread_create(&printer, NULL, printer_thread, NULL);
    
    // 创建用户输入线程
    pthread_t user_input;
    pthread_create(&user_input, NULL, user_input_thread, NULL);
    
    // 生成测试数据
    printf("正在生成5个测试打印任务...\n");
    generate_test_jobs(5);
    
    // 等待线程结束 (实际上会无限运行)
    pthread_join(printer, NULL);
    pthread_join(user_input, NULL);
    
    return 0;
}

//编译：gcc 10_print_spooler.c -o 10_print_spooler -lpthread