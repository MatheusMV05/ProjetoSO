#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t mutex, writeblock;
int data = 0, rc = 0;

void* leitor(void* arg) {
    int id = *((int*)arg);
    while (1) {
        sem_wait(&mutex);
        rc++;
        if (rc == 1)
            sem_wait(&writeblock);
        sem_post(&mutex);

        printf("Leitor %d leu o valor: %d\n", id, data);
        sleep(1);

        sem_wait(&mutex);
        rc--;
        if (rc == 0)
            sem_post(&writeblock);
        sem_post(&mutex);

        sleep(2);
    }
    return NULL;
}

void* escritor(void* arg) {
    int id = *((int*)arg);
    while (1) {
        sem_wait(&writeblock);
        data++;
        printf("Escritor %d escreveu o valor: %d\n", id, data);
        sleep(2);
        sem_post(&writeblock);
        sleep(3);
    }
    return NULL;
}

int main() {
    pthread_t r1, r2, w1;
    int id1 = 1, id2 = 2, id3 = 1;

    sem_init(&mutex, 0, 1);
    sem_init(&writeblock, 0, 1);

    pthread_create(&r1, NULL, leitor, &id1);
    pthread_create(&r2, NULL, leitor, &id2);
    pthread_create(&w1, NULL, escritor, &id3);

    pthread_join(r1, NULL);
    pthread_join(r2, NULL);
    pthread_join(w1, NULL);

    return 0;
}
