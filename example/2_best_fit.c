#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

// 内存块结构体
typedef struct MemoryBlock {
    int start;              // 起始地址
    int size;               // 块大小
    bool is_allocated;      // 是否已分配
    char process_name[20];  // 进程名（如果已分配）
    struct MemoryBlock* next;
} MemoryBlock;

// 内存管理器结构体
typedef struct {
    MemoryBlock* head;      // 内存块链表头
    int total_memory;       // 总内存大小
} MemoryManager;

// 初始化内存管理器
MemoryManager* init_memory_manager(int total_size) {
    MemoryManager* manager = (MemoryManager*)malloc(sizeof(MemoryManager));
    manager->total_memory = total_size;
    
    // 创建初始空闲块
    manager->head = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    manager->head->start = 0;
    manager->head->size = total_size;
    manager->head->is_allocated = false;
    strcpy(manager->head->process_name, "");
    manager->head->next = NULL;
    
    return manager;
}

// 打印内存状态
void print_memory_status(MemoryManager* manager) {
    printf("\n=== 内存分配状态 ===\n");
    MemoryBlock* current = manager->head;
    
    while (current != NULL) {
        printf("地址: %d-%d, 大小: %d, 状态: %s", 
               current->start, 
               current->start + current->size - 1,
               current->size,
               current->is_allocated ? "已分配" : "空闲");
        
        if (current->is_allocated) {
            printf(", 进程: %s", current->process_name);
        }
        printf("\n");
        current = current->next;
    }
    printf("==================\n");
}

// 最佳适应算法分配内存
bool allocate_memory_best_fit(MemoryManager* manager, int size, const char* process_name) {
    MemoryBlock* current = manager->head;
    MemoryBlock* best_block = NULL;
    int min_diff = INT_MAX;
    
    // 查找最适合的空闲块（最小且足够大的块）
    while (current != NULL) {
        if (!current->is_allocated && current->size >= size) {
            int diff = current->size - size;
            if (diff < min_diff) {
                min_diff = diff;
                best_block = current;
            }
        }
        current = current->next;
    }
    
    if (best_block != NULL) {
        // 找到合适的空闲块
        if (best_block->size == size) {
            // 正好匹配，直接分配
            best_block->is_allocated = true;
            strcpy(best_block->process_name, process_name);
        } else {
            // 需要分割块
            MemoryBlock* new_block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
            new_block->start = best_block->start + size;
            new_block->size = best_block->size - size;
            new_block->is_allocated = false;
            strcpy(new_block->process_name, "");
            new_block->next = best_block->next;
            
            // 更新当前块
            best_block->size = size;
            best_block->is_allocated = true;
            strcpy(best_block->process_name, process_name);
            best_block->next = new_block;
        }
        
        printf("成功为进程 %s 分配 %d 字节内存\n", process_name, size);
        return true;
    }
    
    printf("分配失败：没有足够的连续内存空间为进程 %s 分配 %d 字节\n", process_name, size);
    return false;
}

// 合并相邻的空闲块
void merge_free_blocks(MemoryManager* manager) {
    MemoryBlock* current = manager->head;
    
    while (current != NULL && current->next != NULL) {
        if (!current->is_allocated && !current->next->is_allocated) {
            // 两个相邻的空闲块，进行合并
            MemoryBlock* next_block = current->next;
            current->size += next_block->size;
            current->next = next_block->next;
            free(next_block);
            // 不移动current指针，继续检查是否还能合并
        } else {
            current = current->next;
        }
    }
}

// 释放内存
bool deallocate_memory(MemoryManager* manager, const char* process_name) {
    MemoryBlock* current = manager->head;
    
    // 查找要释放的内存块
    while (current != NULL) {
        if (current->is_allocated && strcmp(current->process_name, process_name) == 0) {
            // 找到要释放的块
            current->is_allocated = false;
            strcpy(current->process_name, "");
            
            printf("成功释放进程 %s 的内存\n", process_name);
            
            // 合并相邻的空闲块
            merge_free_blocks(manager);
            return true;
        }
        current = current->next;
    }
    
    printf("释放失败：未找到进程 %s 的内存分配\n", process_name);
    return false;
}



// 显示空闲内存块
void show_free_blocks(MemoryManager* manager) {
    printf("\n=== 空闲内存块 ===\n");
    MemoryBlock* current = manager->head;
    int free_count = 0;
    int total_free = 0;
    
    while (current != NULL) {
        if (!current->is_allocated) {
            printf("空闲块 %d: 地址 %d-%d, 大小 %d\n", 
                   ++free_count,
                   current->start, 
                   current->start + current->size - 1,
                   current->size);
            total_free += current->size;
        }
        current = current->next;
    }
    
    if (free_count == 0) {
        printf("没有空闲内存块\n");
    } else {
        printf("总空闲内存: %d 字节\n", total_free);
    }
    printf("================\n");
}

// 释放内存管理器
void free_memory_manager(MemoryManager* manager) {
    MemoryBlock* current = manager->head;
    while (current != NULL) {
        MemoryBlock* temp = current;
        current = current->next;
        free(temp);
    }
    free(manager);
}

// 主函数演示
int main() {
    // 初始化1024字节的内存
    MemoryManager* manager = init_memory_manager(1024);
    
    printf("初始化内存管理器，总内存: %d 字节\n", manager->total_memory);
    print_memory_status(manager);
    
    int choice;
    char process_name[20];
    int size;
    
    while (1) {
        printf("\n=== 内存管理系统 (最佳适应算法) ===\n");
        printf("1. 分配内存\n");
        printf("2. 释放内存\n");
        printf("3. 显示内存状态\n");
        printf("4. 显示空闲块\n");
        printf("5. 退出\n");
        printf("请选择操作: ");
        
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                printf("请输入进程名: ");
                scanf("%s", process_name);
                printf("请输入需要分配的内存大小: ");
                scanf("%d", &size);
                allocate_memory_best_fit(manager, size, process_name);
                break;
                
            case 2:
                printf("请输入要释放内存的进程名: ");
                scanf("%s", process_name);
                deallocate_memory(manager, process_name);
                break;
                
            case 3:
                print_memory_status(manager);
                break;
                
            case 4:
                show_free_blocks(manager);
                break;
                
            case 5:
                printf("退出程序\n");
                free_memory_manager(manager);
                return 0;
                
            default:
                printf("无效的选择，请重新输入\n");
        }
    }
    
    return 0;
}

//编译：gcc 2_best_fit_memory.c -o 2_best_fit_memory