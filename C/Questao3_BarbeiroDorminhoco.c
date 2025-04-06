#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define CADEIRAS 3

sem_t clientes, barbeiro;
pthread_mutex_t mutex;
int esperando = 0;

void* func_cliente(void* arg) {
    int id = *((int*)arg);
    while (1) {
        pthread_mutex_lock(&mutex);
        if (esperando < CADEIRAS) {
            esperando++;
            printf("Cliente %d esperando. Total: %d\n", id, esperando);
            sem_post(&clientes);
            pthread_mutex_unlock(&mutex);
            sem_wait(&barbeiro);
            printf("Cliente %d cortando o cabelo\n", id);
            sleep(1);
        } else {
            printf("Cliente %d foi embora, sem cadeiras.\n", id);
            pthread_mutex_unlock(&mutex);
        }
        sleep(rand() % 3 + 1);
    }
    return NULL;
}

void* func_barbeiro(void* arg) {
    while (1) {
        sem_wait(&clientes);
        pthread_mutex_lock(&mutex);
        esperando--;
        printf("Barbeiro cortando cabelo. Esperando agora: %d\n", esperando);
        pthread_mutex_unlock(&mutex);
        sleep(2);
        sem_post(&barbeiro);
    }
    return NULL;
}

int main() {
    pthread_t barbeiro_t, clientes_t[5];
    int id[5];

    sem_init(&clientes, 0, 0);
    sem_init(&barbeiro, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&barbeiro_t, NULL, func_barbeiro, NULL);

    for (int i = 0; i < 5; i++) {
        id[i] = i + 1;
        pthread_create(&clientes_t[i], NULL, func_cliente, &id[i]);
    }

    for (int i = 0; i < 5; i++)
        pthread_join(clientes_t[i], NULL);

    return 0;
}
