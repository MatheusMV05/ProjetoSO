#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t flag_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t flag_cv = PTHREAD_COND_INITIALIZER;
int flag = 0;

void* worker(void* arg) {
    int id = *(int*)arg;
    free(arg);
    pthread_mutex_lock(&flag_mutex);
    while (!flag) {  // Aguarda a flag ser ativada
        pthread_cond_wait(&flag_cv, &flag_mutex);
    }
    pthread_mutex_unlock(&flag_mutex);
    printf("Thread %d: Flag ativada, prosseguindo...\n", id);
    return NULL;
}

int main() {
    pthread_t threads[3];
    for (int i = 0; i < 3; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        if (pthread_create(&threads[i], NULL, worker, id) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    sleep(2); // Simula algum processamento
    pthread_mutex_lock(&flag_mutex);
    flag = 1;
    pthread_cond_broadcast(&flag_cv);  // Desbloqueia todas as threads
    pthread_mutex_unlock(&flag_mutex);
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Main: Todas as threads finalizaram.\n");
    return 0;
}
