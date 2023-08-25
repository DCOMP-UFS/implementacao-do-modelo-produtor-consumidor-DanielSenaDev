/* File:  
 *    pth_pool.c
 *
 * Purpose:
 *    Implementação de um pool de threads
 *
 *
 * Compile:  gcc -g -Wall -o pool_rvet pool_rvet.c -lpthread -lrt
 * Usage:    ./pool_rvet
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define THREAD_NUM 6    // Tamanho do pool de threads
#define BUFFER_SIZE 6 // Númermo máximo de tarefas enfileiradas


typedef struct Clock { 
   int p[3];
} Clock;

Clock taskQueue[BUFFER_SIZE];
int taskCount = 0;

pthread_mutex_t mutex;

pthread_cond_t condFull;
pthread_cond_t condEmpty;

void executeTask(Clock* clock, int id){
  printf("%d %d %d executou\n",clock->p[0],clock->p[1],clock->p[2]);
}

Clock getTask(){
   pthread_mutex_lock(&mutex);
   
   while (taskCount == 0){
        printf("vazio\n");
      pthread_cond_wait(&condEmpty, &mutex);
   }
   
   Clock clock = taskQueue[0];
   int i;
   for (i = 0; i < taskCount - 1; i++){
      taskQueue[i] = taskQueue[i+1];
   }
   taskCount--;
   
   pthread_mutex_unlock(&mutex);
   pthread_cond_signal(&condFull);
   return clock;
}

void submitTask(Clock clock){
   pthread_mutex_lock(&mutex);

   while (taskCount == BUFFER_SIZE){
      printf("cheio\n");
      pthread_cond_wait(&condFull, &mutex);
   }

   taskQueue[taskCount] = clock;
   taskCount++;

   pthread_mutex_unlock(&mutex);
   pthread_cond_signal(&condEmpty);
}

void *startProducerThread(void* args);  

void *startConsumerThread(void* args);

/*--------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
   pthread_mutex_init(&mutex, NULL);
   
   pthread_cond_init(&condEmpty, NULL);
   pthread_cond_init(&condFull, NULL);

   pthread_t thread[THREAD_NUM]; 
   srand(time(NULL));
   long i;
   for (int i = 0; i < THREAD_NUM; i++){  
       if(i%2==0)
      if (pthread_create(&thread[i], NULL, &startProducerThread, (void*) i) != 0) {
         perror("Failed to create the thread");
      }  
      else
       if (pthread_create(&thread[i], NULL, &startConsumerThread, (void*) i) != 0) {
         perror("Failed to create the thread");
      } 
   }
   
   
   
   for (int i = 0; i < THREAD_NUM; i++){  
      if (pthread_join(thread[i], NULL) != 0) {
         perror("Failed to join the thread");
      }  
   }
   
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&condEmpty);
   pthread_cond_destroy(&condFull);
   return 0;
}  /* main */

/*-------------------------------------------------------------------*/
void *startProducerThread(void* args) {
   long id = (long) args; 
   for(int i=0;i<50;i++){
      Clock clock = {rand()%100,rand()%100,rand()%100};
      printf("produziu %d %d %d \n",clock.p[0],clock.p[1],clock.p[2]);
      submitTask(clock);
      sleep(rand()%5);
   }
   return NULL;
} 
void *startConsumerThread(void* args) {
   long id = (long) args; 
   for(int i=0;i<50;i++){
      Clock clock = getTask();
      executeTask(&clock, id);
      sleep(rand()%2);
   }
   return NULL;
} 

