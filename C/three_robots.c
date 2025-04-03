#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

sem_t semBart, semLisa1, semMaggie, semLisa2;

void* bart(void* arg) {
    while (1) {
        sem_wait(&semBart);
        printf("Bart executando...\n");
        sleep(1);
        sem_post(&semLisa1);
    }
    return NULL;
}

void* lisa(void* arg) {
    while (1) {
        sem_wait(&semLisa1);
        printf("Lisa executando - turno 1...\n");
        sleep(1);
        sem_post(&semMaggie);
        sem_wait(&semLisa2);
        printf("Lisa executando - turno 2...\n");
        sleep(1);
        sem_post(&semBart);
    }
    return NULL;
}

void* maggie(void* arg) {
    while (1) {
        sem_wait(&semMaggie);
        printf("Maggie executando...\n");
        sleep(1);
        sem_post(&semLisa2);
    }
    return NULL;
}

int main() {
    pthread_t tBart, tLisa, tMaggie;
    sem_init(&semBart, 0, 1);    // Bart inicia apto a executar
    sem_init(&semLisa1, 0, 0);
    sem_init(&semMaggie, 0, 0);
    sem_init(&semLisa2, 0, 0);

    pthread_create(&tBart, NULL, bart, NULL);
    pthread_create(&tLisa, NULL, lisa, NULL);
    pthread_create(&tMaggie, NULL, maggie, NULL);

    pthread_join(tBart, NULL);
    pthread_join(tLisa, NULL);
    pthread_join(tMaggie, NULL);
    return 0;
}
