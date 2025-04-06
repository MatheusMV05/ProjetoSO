#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 5

sem_t forks[N];

void* filosofo(void* arg) {
    int id = *((int*)arg);

    while (1) {
        printf("Filósofo %d pensando...\n", id);
        sleep(1);

        sem_wait(&forks[id]);
        sem_wait(&forks[(id+1)%N]);

        printf("Filósofo %d comendo...\n", id);
        sleep(2);

        sem_post(&forks[id]);
        sem_post(&forks[(id+1)%N]);

        printf("Filósofo %d terminou de comer.\n", id);
    }
    return NULL;
}

int main() {
    pthread_t filosofos[N];
    int id[N];

    for (int i = 0; i < N; i++) {
        sem_init(&forks[i], 0, 1);
        id[i] = i;
        pthread_create(&filosofos[i], NULL, filosofo, &id[i]);
    }

    for (int i = 0; i < N; i++)
        pthread_join(filosofos[i], NULL);

    return 0;
}
