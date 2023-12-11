#include "ppos.h"
#include "ppos-core-globals.h"

// ****************************************************************************
// Coloque aqui as suas modificações, p.ex. includes, defines variáveis,
// estruturas e funções


#include <signal.h>
#include <sys/time.h>
#include <limits.h>

#define TICK_TIME 1
#define QUANTUM_SIZE 20

int currentTaskTime;
struct sigaction action;
struct itimerval timer;

task_t* taskIdle;   // Ponteiro para a TCB da tarefa ociosa
char* taskIdleName = "taskIdle";

void task_idle_body() {
    while (1);
}



void task_set_eet(task_t* task, int et)
{
    if (!task) {
        taskExec->eet = et;
        taskExec->running_time = 0;
    } else {
        task->eet = et;
        task->running_time = 0;
    }
}

int task_get_eet(task_t* task)
{
    if (!task)
        return taskExec->eet;
    else
        return task->eet;
}

int task_get_ret(task_t* task)
{
    if (!task)
        return taskExec->eet - taskExec->running_time;
    else
        return task->eet - task->running_time;
}

void task_setprio(task_t* task, int prio)
{

}

int task_getprio(task_t* task)
{
    return 0;
}

// ****************************************************************************

/* função que tratará os sinais recebidos */
void tratador(int signum)
{
    taskExec->running_time += TICK_TIME;
    currentTaskTime++;
    systemTime += TICK_TIME;

    task_t* next = readyQueue;


    if (currentTaskTime >= QUANTUM_SIZE && taskExec->id != taskDisp->id && taskExec->id != 40)
    {
        currentTaskTime = 0;
        task_yield();
    }

}

void before_ppos_init()
{
    // registra a ação para o sinal de timer SIGALRM
    action.sa_handler = tratador;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGALRM, &action, 0) < 0)
    {
        perror("Erro em sigaction: ");
        exit(1);
    }

    // ajusta valores do temporizador
    timer.it_value.tv_usec = TICK_TIME * 1000;    // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec = 0;        // primeiro disparo, em segundos
    timer.it_interval.tv_usec = TICK_TIME * 1000; // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec = 0;     // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer(ITIMER_REAL, &timer, 0) < 0)
    {
        perror("Erro em setitimer: ");
        exit(1);
    }

#ifdef DEBUG
    printf("\ninit - BEFORE");
#endif
}

void after_ppos_init()
{
    // put your customization here
    task_set_eet(taskMain, INT_MAX / 2);

    // task_create (taskIdle, task_idle_body, &taskIdleName);
#ifdef DEBUG
    printf("\ninit - AFTER");
#endif
}

void before_task_create(task_t* task)
{

    task_set_eet(task, 99999);
    task->start_time = systime();
    // task->suspended = 0;
#ifdef DEBUG
    printf("\ntask_create - BEFORE - [%d]", task->id);
#endif
}

void after_task_create(task_t* task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_create - AFTER - [%d]", task->id);
#endif
}

void before_task_exit()
{
    // put your customization here
    printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", taskExec->id, systime() - taskExec->start_time, taskExec->running_time, taskExec->activations);
    if (task_id() == 0)
        disk_mgr_destroy();
#ifdef DEBUG
    printf("\ntask_exit - BEFORE - [%d]", taskExec->id);
#endif
}

void after_task_exit()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_exit - AFTER- [%d]", taskExec->id);
#endif
}

void before_task_switch(task_t* task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_switch - BEFORE - [%d -> %d]", taskExec->id, task->id);
#endif
}

void after_task_switch(task_t* task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_switch - AFTER - [%d -> %d]", taskExec->id, task->id);
#endif
}

void before_task_yield()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_yield - BEFORE - [%d]", taskExec->id);
#endif
}
void after_task_yield()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_yield - AFTER - [%d]", taskExec->id);
#endif
}

void before_task_suspend(task_t* task)
{
#ifdef DEBUG
    printf("\ntask_suspend - BEFORE - [%d]", task->id);
#endif
}

void after_task_suspend(task_t* task)
{
    // put your customization here
    // task->suspended = 1;
#ifdef DEBUG
    printf("\ntask_suspend - AFTER - [%d]", task->id);
#endif
}

void before_task_resume(task_t* task)
{
    // put your customization here
    // printf("before resuming task\n");
#ifdef DEBUG
    printf("\ntask_resume - BEFORE - [%d]", task->id);
#endif
}

void after_task_resume(task_t* task)
{
    // put your customization here
    // printf("after resuming task\n");

#ifdef DEBUG
    printf("\ntask_resume - AFTER - [%d]", task->id);
#endif
}

void before_task_sleep()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_sleep - BEFORE - [%d]", taskExec->id);
#endif
}

void after_task_sleep()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_sleep - AFTER - [%d]", taskExec->id);
#endif
}

int before_task_join(task_t* task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_join - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_task_join(task_t* task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_join - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_create(semaphore_t* s, int value)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_create(semaphore_t* s, int value)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_down(semaphore_t* s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_down - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_down(semaphore_t* s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_down - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_up(semaphore_t* s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_up - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_up(semaphore_t* s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_up - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_destroy(semaphore_t* s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_destroy(semaphore_t* s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_create(mutex_t* m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_create(mutex_t* m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_lock(mutex_t* m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_lock - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_lock(mutex_t* m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_lock - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_unlock(mutex_t* m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_unlock - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_unlock(mutex_t* m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_unlock - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_destroy(mutex_t* m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_destroy(mutex_t* m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_create(barrier_t* b, int N)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_create(barrier_t* b, int N)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_join(barrier_t* b)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_join - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_join(barrier_t* b)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_join - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_destroy(barrier_t* b)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_destroy(barrier_t* b)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_create(mqueue_t* queue, int max, int size)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_create(mqueue_t* queue, int max, int size)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_send(mqueue_t* queue, void* msg)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_send - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_send(mqueue_t* queue, void* msg)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_send - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_recv(mqueue_t* queue, void* msg)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_recv - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_recv(mqueue_t* queue, void* msg)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_recv - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_destroy(mqueue_t* queue)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_destroy(mqueue_t* queue)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_msgs(mqueue_t* queue)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_msgs - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_msgs(mqueue_t* queue)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_msgs - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

task_t* scheduler()
{

    task_t* nextTask = readyQueue, * chosenTask = NULL;

    //if(!nextTask) return taskIdle;

     // FCFS scheduler
    int minRemaingTime = __INT_MAX__;
    // printf("\nTASKS ==============================\n");
    // printf("task dispatcher: %d, rt %d, count: %d\n\n", taskDisp->id, taskDisp->running_time, countTasks);

    for (int i = 0; i < countTasks; i++) {
        // printf("task %d, runningt %d, remt %d\n", nextTask->id, nextTask->running_time, task_get_ret(nextTask));
        if (task_get_ret(nextTask) < minRemaingTime)
        {
            chosenTask = nextTask;
            minRemaingTime = task_get_ret(chosenTask);
        }
        nextTask = nextTask->next;
    }
    // printf("==============================\n\n");
    if (!chosenTask) {
        printf("oh no no tasks :(\n");
        return taskMain;
    }

    chosenTask->activations++;

    return chosenTask;
}
