#include "ppos_disk.h"
#include "stdio.h"
#include "disk.h"
#include <stdlib.h>


disk_t disk;

void free_disk_handler(int signum)
{
    printf("\n\n\nresuming\n\n\n");
    task_resume(disk.waitingTasks);
}

int disk_mgr_init (int *numBlocks, int *blockSize) {
    
    disk.waitingTasks = NULL;
    if(disk_cmd(DISK_CMD_INIT, 0, 0)) return -1;

    *numBlocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
    *blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);
    
    if(*numBlocks < 0 || *blockSize < 0) return -1;

    disk.ready.sa_handler = free_disk_handler;
    sigemptyset(&disk.ready.sa_mask);
    disk.ready.sa_flags = 0;
    if (sigaction(SIGUSR1, &disk.ready, 0) < 0)
    {
        perror("Erro em sigaction: ");
        exit(1);
    }
    
    return 0;
}

int disk_block_read(int block, void *buffer) {
    printf("BEFORE DISK READ\n");
    int result = disk_cmd(DISK_CMD_STATUS, 0, 0);

    if(result != 1) {
        task_suspend(taskExec, &disk.waitingTasks);
                task_yield();

    }
    printf("running read\n");
    return disk_cmd(DISK_CMD_READ, block, buffer);
    
}

int disk_block_write(int block, void *buffer) {
    printf("BEFORE DISK WRITE\n");

    int result = disk_cmd(DISK_CMD_STATUS, 0, 0);

    if(result != 1) {
        task_suspend(taskExec, &disk.waitingTasks);
        task_yield();
    }
  
    printf("running write\n");
    return disk_cmd(DISK_CMD_WRITE, block, buffer);
}