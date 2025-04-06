#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_ITENS 10

int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;

// Semáforos
sem_t cheio;
sem_t vazio;
pthread_mutex_t mutex;

void* produtor(void* arg) {
    for (int i = 0; i < NUM_ITENS; i++) {
        int item = rand() % 100;

        sem_wait(&vazio);
        pthread_mutex_lock(&mutex);

        buffer[in] = item;
        printf("Produtor produziu: %d na posição %d\n", item, in);
        in = (in + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex);
        sem_post(&cheio);

        sleep(1);  // Simula tempo de produção
    }
    pthread_exit(NULL);
}

void* consumidor(void* arg) {
    for (int i = 0; i < NUM_ITENS; i++) {
        sem_wait(&cheio);
        pthread_mutex_lock(&mutex);

        int item = buffer[out];
        printf("Consumidor consumiu: %d da posição %d\n", item, out);
        out = (out + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex);
        sem_post(&vazio);

        sleep(2);  // Simula tempo de consumo
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t tprodutor, tconsumidor;

    sem_init(&cheio, 0, 0);
    sem_init(&vazio, 0, BUFFER_SIZE);
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&tprodutor, NULL, produtor, NULL);
    pthread_create(&tconsumidor, NULL, consumidor, NULL);

    pthread_join(tprodutor, NULL);
    pthread_join(tconsumidor, NULL);

    sem_destroy(&cheio);
    sem_destroy(&vazio);
    pthread_mutex_destroy(&mutex);

    return 0;
}
