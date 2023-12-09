#include "ppos_disk.h"
#include "stdio.h"
#include "disk.h"
#include <stdlib.h>
#include <limits.h>

disk_t disk;
task_t taskDiskManager;   // Representa a funcao gerenciadora de disco

void task_disk_manager_body()
{
    printf("outside while\n");
    while (1) {
        printf("begin loop\n");
        while (disk_cmd(DISK_CMD_STATUS, 0, 0) != 1 || !disk.waitingOrders) {
            // printf("disk suss, status: %d\n", disk_cmd(DISK_CMD_STATUS, 0, 0));
            task_yield();
        }
        printf("saindo do while\n");
        order* chosen_order;
        printf("dequeuing order status:%d\n", disk_cmd(DISK_CMD_STATUS, 0, 0));
        dequeue_order(&chosen_order, &disk.waitingOrders);
        printf("chosed order type %d, block %d, buffer %d\n", chosen_order->type, chosen_order->block, chosen_order->buffer);

        if (chosen_order->type == 0) {
            printf("chosen order 0\n");
            disk_cmd(DISK_CMD_READ, chosen_order->block, chosen_order->buffer);
        } else {
            printf("chosen order 0\n");
            disk_cmd(DISK_CMD_WRITE, chosen_order->block, chosen_order->buffer);
        }
        printf("resuming task\n");

        // task_resume(chosen_order->caller);
        chosen_order->caller->suspended = 0;
        printf("free thing\n");
        free(chosen_order);
    }
}

void free_disk_handler(int signum)
{
    printf("aaaa\n");
    //taskDiskManager->suspended = 0;
}

void print_queue(order* queue) {

    if (!queue) {
        printf("fila vazia in print\n");
        return;
    }

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
    //printf("enqueueing %d\n", e_order);

    //printf("before enqueue\n");
    // print_queue(*queue);
    if (!e_order) return -1;

    if (!queue || !*queue)
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

    //printf("after enqueue\n");
    //print_queue(*queue);
    return 0;
}

int dequeue_order(order** e_order, order** queue)
{

    //printf("before dequeue\n");
    //print_queue(*queue);
    printf("dequeuing enter\n");
    if (!queue || !(*queue))
    {
        printf("fila vazia\n");
        return -1;
    }

    *e_order = (*queue)->next;
    if ((*queue)->next == (*queue)->prev) {
        printf("1 or 2\n");
        if ((*queue)->next == *queue) {
            printf("equeuing order\n");
            *queue = NULL;
        } else
        {
            printf("2\n");
            (*queue)->next = *queue;
            printf("next\n");

            (*queue)->prev = *queue;
            printf("prev\n");
        }
    } else
    {
        (*queue)->next->next->prev = (*queue);
        (*queue)->next = (*queue)->next->next;
    }
    printf("dequeuing done %d\n", *e_order);

    //printf("after dequeue\n");
    //print_queue(*queue);
}

int disk_mgr_init(int* numBlocks, int* blockSize)
{

    disk.waitingTasks = NULL;
    if (disk_cmd(DISK_CMD_INIT, 0, 0))
        return -1;

    *numBlocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
    *blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);

    task_create(&taskDiskManager, task_disk_manager_body, "disco");
    task_setprio(&taskDiskManager, 0);
    task_set_eet(&taskDiskManager, INT_MAX - 1);
    taskDiskManager.id = 40;

    if (*numBlocks < 0 || *blockSize < 0)
        return -1;

    /* disk.ready.sa_handler = free_disk_handler;
    sigemptyset(&disk.ready.sa_mask);
    disk.ready.sa_flags = 0;
    if (sigaction(SIGUSR1, &disk.ready, 0) < 0)
    {
        perror("Erro em sigaction: ");
        exit(1);
    } */

    if (signal(SIGUSR1, free_disk_handler) == SIG_ERR) {
        perror("Erro em sig: ");
        exit(1);
    }


    return 0;
}

int disk_block_read(int block, void* buffer)
{
    /* printf("BEFORE DISK READ\n");
    int result = disk_cmd(DISK_CMD_STATUS, 0, 0);

    if(result != 1) {
        task_suspend(taskExec, &disk.waitingTasks);
        task_yield();

    }
    printf("running read\n");
    return disk_cmd(DISK_CMD_READ, block, buffer); */

    order* new_read_order;
    new_read_order = (order*)malloc(sizeof(order));
    new_read_order->caller = taskExec;
    new_read_order->type = 0;
    new_read_order->block = block;
    new_read_order->buffer = buffer;
    new_read_order->prev = NULL;
    new_read_order->next = NULL;

    printf("equeuing order\n");
    enqueue_order(new_read_order, &disk.waitingOrders);

    // task_suspend(taskExec, &disk.waitingTasks);
    taskExec->suspended = 1;
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

    printf("equeuing order\n");
    enqueue_order(new_write_order, &disk.waitingOrders);

    // task_suspend(taskExec, &disk.waitingTasks);
    taskExec->suspended = 1;
    task_yield();

    return 0;
    // printf("BEFORE DISK WRITE\n");

    // int result = disk_cmd(DISK_CMD_STATUS, 0, 0);

    // if(result != 1) {
    //     task_suspend(taskExec, &disk.waitingTasks);
    //     task_yield();
    // }

    // printf("running write\n");
    // return disk_cmd(DISK_CMD_WRITE, block, buffer);
}

