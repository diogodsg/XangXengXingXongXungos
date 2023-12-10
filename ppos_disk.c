#include "ppos_disk.h"
#include "stdio.h"
#include "disk.h"
#include <stdlib.h>
#include <limits.h>

disk_t disk;
task_t taskDiskManager;   // Representa a funcao gerenciadora de disco
// mutex_t mutex;
task_t* command_running_task;
mqueue_t order_queue;
task_t* sussy = NULL;
int active;

void task_disk_manager_body()
{

    while (active) {
        while (disk.waitingOrders == NULL) task_yield();

        order* chosen_order = NULL;

        dequeue_order(&chosen_order, &disk.waitingOrders);

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

int dequeue_order(order** e_order, order** queue)
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

void disk_mgr_destroy() {
    active = 0;
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

