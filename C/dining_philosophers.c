#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_PHILOS 5

sem_t chopsticks[NUM_PHILOS];
sem_t saleiro;

void* philosopher(void* arg) {
    int id = *(int*)arg;
    free(arg);
    int left = id;
    int right = (id + 1) % NUM_PHILOS;
    while (1) {
        printf("Filósofo %d está meditando.\n", id);
        sleep(1);
        sem_wait(&saleiro);  // Pega o saleiro
        sem_wait(&chopsticks[left]);
        sem_wait(&chopsticks[right]);
        printf("Filósofo %d está comendo.\n", id);
        sleep(2);
        sem_post(&chopsticks[left]);
        sem_post(&chopsticks[right]);
        sem_post(&saleiro);
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_PHILOS];
    sem_init(&saleiro, 0, NUM_PHILOS - 1); // Permite que no máximo 4 filósofos tentem comer
    for (int i = 0; i < NUM_PHILOS; i++) {
        sem_init(&chopsticks[i], 0, 1);
    }
    for (int i = 0; i < NUM_PHILOS; i++) {
        int* id = malloc(sizeof(int));
        *id = i;
        pthread_create(&threads[i], NULL, philosopher, id);
    }
    for (int i = 0; i < NUM_PHILOS; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
