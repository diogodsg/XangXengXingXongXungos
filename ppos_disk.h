// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.2 -- Julho de 2017

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__

#include <signal.h>
#include "ppos.h"
#include "ppos_data.h"
#include "ppos-core-globals.h"
// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

// estrutura que representa um disco no sistema operacional
typedef struct order_t
{
  struct order_t* prev, * next; // ponteiros para usar em filas
  int block;
  void* buffer;
  int type; //0 = read; 1 = write
  struct task_t* caller;



} order;

typedef struct
{
  // completar com os campos necessarios
  int numblocks;		// numero de blocos do disco
  int blocksize;		// tamanho dos blocos em bytes
  task_t* waitingTasks;
  order* waitingOrders;
  struct sigaction ready;	// tratador de sinal de libeda
} disk_t;



// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init(int* numBlocks, int* blockSize);
void disk_mgr_destroy();
// leitura de um bloco, do disco para o buffer
int disk_block_read(int block, void* buffer);

// escrita de um bloco, do buffer para o disco
int disk_block_write(int block, void* buffer);


int enqueue_order(order* e_order, order** queue);
int dequeue_order(order** e_order, order** queue);



#endif
