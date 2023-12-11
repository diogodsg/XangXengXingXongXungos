#include "ppos_disk.h"
#include "stdio.h"
#include "disk.h"
#include <stdlib.h>
#include <limits.h>

disk_t disk;
task_t taskDiskManager;   // Representa a funcao gerenciadora de disco
// mutex_t mutex;
int headPos = 0;
task_t* command_running_task;
mqueue_t order_queue;
task_t* sussy = NULL;
int active;
int wanderedBlocks = 0;
void task_disk_manager_body()
{

    while (active) {
        while (disk.waitingOrders == NULL) task_yield();

        order* chosen_order = NULL;
        int lastBlock = headPos;

#ifdef CSCAN    
        dequeue_order_cscan(&chosen_order, &disk.waitingOrders);
#elif SSTF
        dequeue_order_sstf(&chosen_order, &disk.waitingOrders);
#else
        dequeue_order_sstf(&chosen_order, &disk.waitingOrders);
#endif
        if (headPos >= lastBlock) {
            wanderedBlocks += abs(lastBlock - headPos);
        } else {
            wanderedBlocks += 256 - lastBlock + headPos;
        }
        task_suspend(&taskDiskManager, NULL);
        if (chosen_order->type == 0) {
            disk_cmd(DISK_CMD_READ, chosen_order->block, chosen_order->buffer);
        } else {
            disk_cmd(DISK_CMD_WRITE, chosen_order->block, chosen_order->buffer);
        }


        task_yield();

        task_resume(chosen_order->caller);

        free(chosen_order);
    }

    task_exit(0);
}

void free_disk_handler(int signum)
{
    task_resume(&taskDiskManager);
}

void print_queue(order* queue) {
    printf("");
    if (!queue) return;

    order* next = queue;
    printf("[%d]", next);
    next = queue->prev;
    while (next != queue) {
        printf("->[%d]", next);
        next = next->prev;
    }

    printf("\n");
}

int enqueue_order(order* e_order, order** queue)
{
    if (!e_order || !queue) return -1;

    if (!(*queue))
    {
        *queue = e_order;

        e_order->next = e_order;
        e_order->prev = e_order;
        return 0;
    }

    order* first = (*queue)->next;
    (*queue)->next = e_order;
    (*queue)->next->next = first;
    (*queue)->next->prev = (*queue);
    first->prev = e_order;
    *queue = e_order;

    return 0;
}

int dequeue_order_fcfs(order** e_order, order** queue)
{

    if (!queue || !(*queue)) {
        return -1;
    }

    *e_order = (*queue)->next;
    if ((*queue)->next == (*queue)->prev) {
        if ((*queue)->next == *queue) {
            *queue = NULL;
        } else
        {
            (*queue)->next = *queue;

            (*queue)->prev = *queue;
        }
    } else
    {
        (*queue)->next->next->prev = (*queue);
        (*queue)->next = (*queue)->next->next;
    }
}

int dequeue_order_sstf(order** e_order, order** queue)
{
    if (!queue || !(*queue)) {
        return -1;
    }


    order* next = queue;
    order* chosenTask = queue;

    int closest = INT_MAX;

    next = (*queue);
    int count = 0;
    do {
        count++;
        if (abs(headPos - next->block) < closest) {
            closest = next->block;
            chosenTask = next;
        }
        next = next->next;
    } while (next != (*queue));

    headPos = chosenTask->block;


    chosenTask->prev->next = chosenTask->next;
    chosenTask->next->prev = chosenTask->prev;

    *e_order = chosenTask;
    if (count == 1) (*queue) = NULL;
    else
        (*queue) = (*e_order)->next;

    return 0;

}


int dequeue_order_cscan(order** e_order, order** queue)
{
    if (!queue || !(*queue)) {
        return -1;
    }
    order* next = *queue;
    order* chosenTask = NULL;

    int closest = INT_MAX;

    int count = 0;

    while (closest > 256) {
        do {
            count++;

            if (next->block >= headPos && next->block - headPos <= closest) {
                closest = next->block - headPos;
                chosenTask = next;
            }

            next = next->next;
        } while (next != (*queue));

        if (!chosenTask) {
            closest = INT_MAX;
            headPos = 0;
            count = 0;
        }
    }

    headPos = chosenTask->block;


    chosenTask->prev->next = chosenTask->next;
    chosenTask->next->prev = chosenTask->prev;
    *e_order = chosenTask;
    if (count == 1) (*queue) = NULL;
    else
        (*queue) = (*e_order)->next;

    return 0;

}


void disk_mgr_destroy() {
#ifdef CSCAN    
    printf("wandered blocks using CSCAN: %d blocks\n", wanderedBlocks);
#elif SSTF
    printf("wandered blocks using SSTF: %d blocks\n", wanderedBlocks);
#else
    printf("wandered blocks using FCFS: %d blocks\n", wanderedBlocks);
#endif    
    exit(1);
}

int disk_mgr_init(int* numBlocks, int* blockSize)
{
    active = 1;
    disk.waitingTasks = NULL;
    if (disk_cmd(DISK_CMD_INIT, 0, 0))
        return -1;

    *numBlocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
    *blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);

    task_create(&taskDiskManager, task_disk_manager_body, "disco");
    task_setprio(&taskDiskManager, 0);
    task_set_eet(&taskDiskManager, INT_MAX - 1);
    taskDiskManager.id = 40;

    mqueue_create(&order_queue, 16, sizeof(order*));

    if (*numBlocks < 0 || *blockSize < 0)
        return -1;

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

int disk_block_read(int block, void* buffer)
{
    order* new_read_order;
    new_read_order = (order*)malloc(sizeof(order));
    new_read_order->caller = taskExec;
    new_read_order->type = 0;
    new_read_order->block = block;
    new_read_order->buffer = buffer;
    new_read_order->prev = NULL;
    new_read_order->next = NULL;

    taskExec->forbid_preempt = 1;
    enqueue_order(new_read_order, &disk.waitingOrders);
    task_suspend(taskExec, &disk.waitingTasks);

    task_yield();


    return 0;
}

int disk_block_write(int block, void* buffer)
{

    order* new_write_order;
    new_write_order = (order*)malloc(sizeof(order));
    new_write_order->caller = taskExec;
    new_write_order->type = 1;
    new_write_order->block = block;
    new_write_order->buffer = buffer;
    new_write_order->prev = NULL;
    new_write_order->next = NULL;

    enqueue_order(new_write_order, &disk.waitingOrders);

    task_suspend(taskExec, &disk.waitingTasks);
    task_yield();

    return 0;
}

