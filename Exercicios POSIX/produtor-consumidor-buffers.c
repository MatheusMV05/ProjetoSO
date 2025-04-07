/*
 * Solução para o Exercício 1:
 * Sistema produtor/consumidor com dois buffers limitados organizados na forma X → B1 → Y → B2 → Z
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <pthread.h>
 #include <semaphore.h>
 #include <unistd.h>
 
 // Capacidades dos buffers
 #define N1 5  // Capacidade do buffer B1
 #define N2 3  // Capacidade do buffer B2
 
 // Estrutura para representar um buffer
 typedef struct {
     int *itens;       // Array para armazenar os itens
     int capacidade;   // Capacidade máxima do buffer
     int inicio;       // Índice do primeiro item
     int fim;          // Índice do próximo espaço livre
     int count;        // Número de itens no buffer
 } Buffer;
 
 // Buffers compartilhados
 Buffer B1, B2;
 
 // Semáforos para o buffer B1
 sem_t vaga_B1;     // Controla as vagas disponíveis em B1 (inicia em N1)
 sem_t item_B1;     // Controla os itens disponíveis em B1 (inicia em 0)
 pthread_mutex_t mutex_B1 = PTHREAD_MUTEX_INITIALIZER;  // Controla o acesso a B1
 
 // Semáforos para o buffer B2
 sem_t vaga_B2;     // Controla as vagas disponíveis em B2 (inicia em N2)
 sem_t item_B2;     // Controla os itens disponíveis em B2 (inicia em 0)
 pthread_mutex_t mutex_B2 = PTHREAD_MUTEX_INITIALIZER;  // Controla o acesso a B2
 
 // Inicializa um buffer
 void inicializar_buffer(Buffer *buffer, int capacidade) {
     buffer->itens = (int*)malloc(capacidade * sizeof(int));
     buffer->capacidade = capacidade;
     buffer->inicio = 0;
     buffer->fim = 0;
     buffer->count = 0;
 }
 
 // Insere um item no buffer
 void insere(Buffer *buffer, int item) {
     buffer->itens[buffer->fim] = item;
     buffer->fim = (buffer->fim + 1) % buffer->capacidade;
     buffer->count++;
 }
 
 // Retira um item do buffer
 int retira(Buffer *buffer) {
     int item = buffer->itens[buffer->inicio];
     buffer->inicio = (buffer->inicio + 1) % buffer->capacidade;
     buffer->count--;
     return item;
 }
 
 // Função do processo X (produtor para B1)
 void* processo_X(void *arg) {
     int item_count = 0;
     
     while (1) {
         // Produz um item
         int item = ++item_count;
         printf("X produziu item %d\n", item);
         
         // Espera uma vaga em B1
         sem_wait(&vaga_B1);
         
         // Acessa B1 exclusivamente
         pthread_mutex_lock(&mutex_B1);
         
         // Insere o item em B1
         insere(&B1, item);
         printf("X inseriu item %d em B1\n", item);
         
         // Libera o acesso a B1
         pthread_mutex_unlock(&mutex_B1);
         
         // Sinaliza que há um novo item em B1
         sem_post(&item_B1);
         
         // Simula o tempo para produzir o próximo item
         sleep(rand() % 3);
     }
     
     return NULL;
 }
 
 // Função do processo Y (consumidor de B1 e produtor para B2)
 void* processo_Y(void *arg) {
     while (1) {
         // Espera um item em B1
         sem_wait(&item_B1);
         
         // Acessa B1 exclusivamente
         pthread_mutex_lock(&mutex_B1);
         
         // Retira o item de B1
         int item = retira(&B1);
         printf("Y retirou item %d de B1\n", item);
         
         // Libera o acesso a B1
         pthread_mutex_unlock(&mutex_B1);
         
         // Sinaliza que há uma nova vaga em B1
         sem_post(&vaga_B1);
         
         // Processa o item
         printf("Y está processando item %d\n", item);
         sleep(1);  // Simula o tempo de processamento
         
         // Espera uma vaga em B2
         sem_wait(&vaga_B2);
         
         // Acessa B2 exclusivamente
         pthread_mutex_lock(&mutex_B2);
         
         // Insere o item processado em B2
         insere(&B2, item);
         printf("Y inseriu item %d em B2\n", item);
         
         // Libera o acesso a B2
         pthread_mutex_unlock(&mutex_B2);
         
         // Sinaliza que há um novo item em B2
         sem_post(&item_B2);
     }
     
     return NULL;
 }
 
 // Função do processo Z (consumidor de B2)
 void* processo_Z(void *arg) {
     while (1) {
         // Espera um item em B2
         sem_wait(&item_B2);
         
         // Acessa B2 exclusivamente
         pthread_mutex_lock(&mutex_B2);
         
         // Retira o item de B2
         int item = retira(&B2);
         printf("Z retirou item %d de B2\n", item);
         
         // Libera o acesso a B2
         pthread_mutex_unlock(&mutex_B2);
         
         // Sinaliza que há uma nova vaga em B2
         sem_post(&vaga_B2);
         
         // Consome o item
         printf("Z está consumindo item %d\n", item);
         sleep(rand() % 4);  // Simula o tempo de consumo
     }
     
     return NULL;
 }
 
 int main() {
     // Inicializar os buffers
     inicializar_buffer(&B1, N1);
     inicializar_buffer(&B2, N2);
     
     // Inicializar os semáforos
     sem_init(&vaga_B1, 0, N1);
     sem_init(&item_B1, 0, 0);
     sem_init(&vaga_B2, 0, N2);
     sem_init(&item_B2, 0, 0);
     
     // Inicializar a semente para números aleatórios
     srand(time(NULL));
     
     // Criar threads para os processos X, Y e Z
     pthread_t thread_X[3];  // 3 produtores X
     pthread_t thread_Y[2];  // 2 processadores Y
     pthread_t thread_Z[3];  // 3 consumidores Z
     
     for (int i = 0; i < 3; i++) {
         pthread_create(&thread_X[i], NULL, processo_X, NULL);
     }
     
     for (int i = 0; i < 2; i++) {
         pthread_create(&thread_Y[i], NULL, processo_Y, NULL);
     }
     
     for (int i = 0; i < 3; i++) {
         pthread_create(&thread_Z[i], NULL, processo_Z, NULL);
     }
     
     // Aguardar um comando para encerrar a simulação
     printf("\nPressione Enter para encerrar a simulação...\n");
     getchar();
     
     // Não precisamos aguardar as threads terminarem, pois vamos encerrar o programa
     
     // Destruir os semáforos
     sem_destroy(&vaga_B1);
     sem_destroy(&item_B1);
     sem_destroy(&vaga_B2);
     sem_destroy(&item_B2);
     
     // Liberar a memória dos buffers
     free(B1.itens);
     free(B2.itens);
     
     return 0;
 }