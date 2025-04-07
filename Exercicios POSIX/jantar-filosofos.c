/*
 * Solução para o Exercício 2:
 * Jantar dos Filósofos com eliminação de impasse
 * 
 * O código original apresenta risco de impasse porque todos os filósofos
 * podem pegar o garfo da direita simultaneamente e depois ficarem bloqueados
 * esperando o da esquerda, que nunca estará disponível.
 * 
 * A solução implementada aqui usa duas abordagens para resolver o impasse:
 * 1. Modificamos a ordem de aquisição dos garfos para filósofos ímpares
 * 2. Adicionamos um tempo de espera aleatório antes de tentar pegar os garfos
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <pthread.h>
 #include <semaphore.h>
 #include <unistd.h>
 
 #define N 5
 
 // Semáforos para os garfos
 sem_t garfo[N];
 
 // Função para simular meditação
 void medita() {
     int tempo = rand() % 3 + 1;
     printf("Filósofo está meditando por %d segundos...\n", tempo);
     sleep(tempo);
 }
 
 // Função para simular ação de comer
 void come(int id) {
     int tempo = rand() % 3 + 1;
     printf("Filósofo %d está comendo por %d segundos...\n", id, tempo);
     sleep(tempo);
 }
 
 // Função executada por cada filósofo
 void* filosofo(void* arg) {
     int id = *((int*)arg);
     int garfo_esquerda = id;
     int garfo_direita = (id + 1) % N;
     
     printf("Filósofo %d iniciou (garfos: %d e %d)\n", id, garfo_esquerda, garfo_direita);
     
     while (1) {
         // Meditar
         medita();
         
         printf("Filósofo %d está com fome\n", id);
         
         // Solução 1: Filósofos com ID par pegam primeiro o garfo da direita,
         // enquanto filósofos com ID ímpar pegam primeiro o da esquerda.
         // Esta ordem diferente quebra a simetria e evita o impasse.
         if (id % 2 == 0) {
             printf("Filósofo %d tentando pegar garfo %d (direita)\n", id, garfo_direita);
             sem_wait(&garfo[garfo_direita]);
             printf("Filósofo %d pegou garfo %d (direita)\n", id, garfo_direita);
             
             // Adiciona um pequeno delay antes de tentar pegar o segundo garfo
             usleep(100000);  // 100ms
             
             printf("Filósofo %d tentando pegar garfo %d (esquerda)\n", id, garfo_esquerda);
             sem_wait(&garfo[garfo_esquerda]);
             printf("Filósofo %d pegou garfo %d (esquerda)\n", id, garfo_esquerda);
         } else {
             printf("Filósofo %d tentando pegar garfo %d (esquerda)\n", id, garfo_esquerda);
             sem_wait(&garfo[garfo_esquerda]);
             printf("Filósofo %d pegou garfo %d (esquerda)\n", id, garfo_esquerda);
             
             // Adiciona um pequeno delay antes de tentar pegar o segundo garfo
             usleep(100000);  // 100ms
             
             printf("Filósofo %d tentando pegar garfo %d (direita)\n", id, garfo_direita);
             sem_wait(&garfo[garfo_direita]);
             printf("Filósofo %d pegou garfo %d (direita)\n", id, garfo_direita);
         }
         
         // Comer
         printf("Filósofo %d começou a comer\n", id);
         come(id);
         printf("Filósofo %d terminou de comer\n", id);
         
         // Soltar os garfos na mesma ordem em que foram pegos
         if (id % 2 == 0) {
             sem_post(&garfo[garfo_esquerda]);
             printf("Filósofo %d liberou garfo %d (esquerda)\n", id, garfo_esquerda);
             sem_post(&garfo[garfo_direita]);
             printf("Filósofo %d liberou garfo %d (direita)\n", id, garfo_direita);
         } else {
             sem_post(&garfo[garfo_direita]);
             printf("Filósofo %d liberou garfo %d (direita)\n", id, garfo_direita);
             sem_post(&garfo[garfo_esquerda]);
             printf("Filósofo %d liberou garfo %d (esquerda)\n", id, garfo_esquerda);
         }
     }
     
     return NULL;
 }
 
 // Função principal
 int main() {
     pthread_t threads[N];
     int ids[N];
     
     // Inicializar semente para números aleatórios
     srand(time(NULL));
     
     // Inicializar semáforos (todos com valor 1)
     for (int i = 0; i < N; i++) {
         sem_init(&garfo[i], 0, 1);
     }
     
     // Criar threads para os filósofos
     for (int i = 0; i < N; i++) {
         ids[i] = i;
         if (pthread_create(&threads[i], NULL, filosofo, &ids[i]) != 0) {
             perror("Erro ao criar thread");
             return 1;
         }
     }
     
     // Aguardar um comando para encerrar a simulação
     printf("\nPressione Enter para encerrar a simulação...\n");
     getchar();
     
     // Não precisamos aguardar as threads terminarem, vamos encerrar o programa
     
     // Destruir os semáforos
     for (int i = 0; i < N; i++) {
         sem_destroy(&garfo[i]);
     }
     
     return 0;
 }
 
