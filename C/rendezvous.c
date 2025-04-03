#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

sem_t sem1, sem2;

void* threadA(void* arg) {
    printf("Thread A chegou no ponto de rendez-vous.\n");
    sem_post(&sem1);   // Sinaliza sua chegada
    sem_wait(&sem2);   // Espera pela Thread B
    printf("Thread A prossegue após o encontro.\n");
    return NULL;
}

void* threadB(void* arg) {
    printf("Thread B chegou no ponto de rendez-vous.\n");
    sem_post(&sem2);   // Sinaliza sua chegada
    sem_wait(&sem1);   // Espera pela Thread A
    printf("Thread B prossegue após o encontro.\n");
    return NULL;
}

int main() {
    pthread_t tA, tB;
    sem_init(&sem1, 0, 0);
    sem_init(&sem2, 0, 0);

    pthread_create(&tA, NULL, threadA, NULL);
    pthread_create(&tB, NULL, threadB, NULL);

    pthread_join(tA, NULL);
    pthread_join(tB, NULL);

    sem_destroy(&sem1);
    sem_destroy(&sem2);
    return 0;
}
