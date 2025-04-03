#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF1_SIZE 5
#define BUF2_SIZE 5
#define NITEMS 20

int buffer1[BUF1_SIZE], in1 = 0, out1 = 0;
int buffer2[BUF2_SIZE], in2 = 0, out2 = 0;

/* Semáforos e mutex para buffer 1 */
sem_t sem_empty1, sem_full1;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

/* Semáforos e mutex para buffer 2 */
sem_t sem_empty2, sem_full2;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* producer(void* arg) {
    for (int i = 0; i < NITEMS; i++) {
        int item = i;
        sem_wait(&sem_empty1);
        pthread_mutex_lock(&mutex1);
        buffer1[in1] = item;
        in1 = (in1 + 1) % BUF1_SIZE;
        printf("Produtor produziu: %d\n", item);
        pthread_mutex_unlock(&mutex1);
        sem_post(&sem_full1);
        sleep(1);
    }
    return NULL;
}

void* intermediate(void* arg) {
    for (int i = 0; i < NITEMS; i++) {
        int item;
        sem_wait(&sem_full1);
        pthread_mutex_lock(&mutex1);
        item = buffer1[out1];
        out1 = (out1 + 1) % BUF1_SIZE;
        pthread_mutex_unlock(&mutex1);
        sem_post(&sem_empty1);

        /* Processamento intermediário: por exemplo, dobrar o valor */
        item = item * 2;

        sem_wait(&sem_empty2);
        pthread_mutex_lock(&mutex2);
        buffer2[in2] = item;
        in2 = (in2 + 1) % BUF2_SIZE;
        printf("Intermediário inseriu no buffer2: %d\n", item);
        pthread_mutex_unlock(&mutex2);
        sem_post(&sem_full2);
        sleep(1);
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 0; i < NITEMS; i++) {
        int item;
        sem_wait(&sem_full2);
        pthread_mutex_lock(&mutex2);
        item = buffer2[out2];
        out2 = (out2 + 1) % BUF2_SIZE;
        pthread_mutex_unlock(&mutex2);
        sem_post(&sem_empty2);
        printf("Consumidor consumiu: %d\n", item);
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t tid_prod, tid_inter, tid_cons;
    sem_init(&sem_empty1, 0, BUF1_SIZE);
    sem_init(&sem_full1, 0, 0);
    sem_init(&sem_empty2, 0, BUF2_SIZE);
    sem_init(&sem_full2, 0, 0);

    pthread_create(&tid_prod, NULL, producer, NULL);
    pthread_create(&tid_inter, NULL, intermediate, NULL);
    pthread_create(&tid_cons, NULL, consumer, NULL);

    pthread_join(tid_prod, NULL);
    pthread_join(tid_inter, NULL);
    pthread_join(tid_cons, NULL);

    sem_destroy(&sem_empty1);
    sem_destroy(&sem_full1);
    sem_destroy(&sem_empty2);
    sem_destroy(&sem_full2);
    return 0;
}
