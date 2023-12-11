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


    if (currentTaskTime >= QUANTUM_SIZE && taskExec->id != taskDisp->id && taskExec->id != 40 && !taskExec->forbid_preempt)
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
    task->forbid_preempt = 0;
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
