#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define BUFFER_SIZE 4096

// 显示用法信息
void print_usage(const char *program_name) {
    printf("用法: %s <源文件> <目标文件>\n", program_name);
    printf("示例: %s /path/to/source.txt /path/to/backup.txt\n", program_name);
}

// 检查文件是否存在
int file_exists(const char *filename) {
    struct stat st;
    return (stat(filename, &st) == 0);
}

// 执行文件备份
int backup_file(const char *source_path, const char *dest_path) {
    FILE *source_file = NULL;
    FILE *dest_file = NULL;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    int ret = 0;

    // 打开源文件
    source_file = fopen(source_path, "rb");
    if (source_file == NULL) {
        fprintf(stderr, "错误: 无法打开源文件 '%s' (%s)\n", 
                source_path, strerror(errno));
        return -1;
    }

    // 打开目标文件
    dest_file = fopen(dest_path, "wb");
    if (dest_file == NULL) {
        fprintf(stderr, "错误: 无法创建目标文件 '%s' (%s)\n", 
                dest_path, strerror(errno));
        fclose(source_file);
        return -1;
    }

    // 复制文件内容
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, source_file)) ){
        size_t bytes_written = fwrite(buffer, 1, bytes_read, dest_file);
        if (bytes_written != bytes_read) {
            fprintf(stderr, "错误: 写入目标文件失败 (%s)\n", strerror(errno));
            ret = -1;
            break;
        }
    }

    // 检查读取错误
    if (ferror(source_file)) {
        fprintf(stderr, "错误: 读取源文件失败 (%s)\n", strerror(errno));
        ret = -1;
    }

    // 关闭文件
    fclose(source_file);
    fclose(dest_file);

    // 复制文件权限
    if (ret == 0) {
        struct stat st;
        if (stat(source_path, &st) == 0) {
            chmod(dest_path, st.st_mode);
        }
    }

    return ret;
}

int main(int argc, char *argv[]) {
    // 检查参数
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char *source_path = argv[1];
    const char *dest_path = argv[2];

    // 检查源文件是否存在
    if (!file_exists(source_path)) {
        fprintf(stderr, "错误: 源文件 '%s' 不存在\n", source_path);
        return 1;
    }

    // 检查目标文件是否已存在
    if (file_exists(dest_path)) {
        printf("警告: 目标文件 '%s' 已存在，将被覆盖\n", dest_path);
        printf("是否继续? (y/n): ");
        char response = getchar();
        if (response != 'y' && response != 'Y') {
            printf("操作已取消\n");
            return 0;
        }
    }

    // 执行备份
    printf("正在备份 '%s' 到 '%s'...\n", source_path, dest_path);
    if (backup_file(source_path, dest_path) == 0) {
        printf("备份成功完成\n");
    } else {
        printf("备份失败\n");
        return 1;
    }

    return 0;
}

//编译：gcc 9_file_backup.c -o 9_file_backup