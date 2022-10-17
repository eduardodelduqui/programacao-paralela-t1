
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/sem.h>
#include <semaphore.h>

/*
* Mutexes
*/
pthread_mutex_t mutex_clientes = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_pagamento = PTHREAD_MUTEX_INITIALIZER;

/*
* Semáforos
*/

sem_t barbearia;
sem_t sofa;
sem_t cliente;
sem_t barbeiro;
sem_t pagamento_cliente;
sem_t pagamento_barbeiro;

#define NUM_MAX_SOFA 3
#define NUM_MAX_TOTAL 20
#define TRUE 1
#define FALSE 0

int contador_fila1 = 0;

void desiste(long tid) {
    printf("Cliente %ld desiste e vai embora\n", tid);
    pthread_exit(NULL);
}

void get_hair_cut (long cliente) {
    printf("Cliente %ld: Cabelo sendo cortado\n", cliente);
    sleep(5);
}

void cut_hair (long barbeiro) {
    printf("Barbeiro %ld: Cabelo sendo cortado\n", barbeiro);
    sleep(3);
}

void aceita_pagamento (long barbeiro) {
    pthread_mutex_lock(&mutex_pagamento);
        sleep(2);
        printf("Barbeiro: %ld: Realiza pagamento\n", barbeiro);
    pthread_mutex_unlock(&mutex_pagamento);
}

void efetua_pagamento (long cliente) {
    printf("Cliente: %ld: Sinaliza pagamento\n", cliente);
}

void *barbeiro_behavior (void *id) {
    long tid;
    tid = (long)id;
    int value;

    printf("Funcionário %ld: chegou na loja\n", tid);
    while(1) {
        if(sem_trywait(&pagamento_cliente) >= 0) {
            aceita_pagamento(tid);
            sem_post(&pagamento_barbeiro);
        };
        if(sem_trywait(&cliente) >= 0) {
            sem_post(&barbeiro);
            cut_hair(tid);
        }
    }
}

void *cliente_behavior(void *id){
    long tid;
    tid = (long)id;
    
    printf("Cliente %ld chegou\n", tid);
    pthread_mutex_lock( &mutex_clientes );
    if (contador_fila1 == NUM_MAX_TOTAL) {
        pthread_mutex_unlock( &mutex_clientes );
        desiste(tid);
    }
    contador_fila1++;
    pthread_mutex_unlock( &mutex_clientes );
    sem_wait(&sofa);
    printf("Cliente %ld sentou no sofa\n", tid);
    sem_post(&cliente);
    sem_wait(&barbeiro);
    sem_post(&sofa);
    printf("Cliente %ld se posiciona para cortar o cabelo\n", tid);
    get_hair_cut(tid);
    sem_post(&pagamento_cliente);
    efetua_pagamento(tid);
    sem_wait(&pagamento_barbeiro);
    printf("Cliente vai embora\n");
}
    

int main(int argc, char *argv[]){
    int num = 30;
    pthread_t thread_clientes[num];
    pthread_t thread_barbeiros[3];

    sem_init(&sofa, 0, NUM_MAX_SOFA);
    sem_init(&pagamento_cliente, 0, 0);
    sem_init(&pagamento_barbeiro, 0, 0);

    /*
    * Variáveis para os loops
    */
    long b, i, t, c;

    for (b = 0; b < 3; b++) {
        sleep(rand() % 3);
        pthread_create(&thread_barbeiros[b], NULL, barbeiro_behavior, (void *)b); 
    }

    for (i = 0; i < num; i++) {
        // sleep(rand() % 3);
        pthread_create(&thread_clientes[i], NULL, cliente_behavior, (void *)i); 
    }

    for(t=0; t< 3; t++){
        pthread_join(thread_barbeiros[t], NULL);
    }

    for(c=0; c< 3; c++){
        pthread_join(thread_clientes[c], NULL);
    }
}