/*
 * Atividade 2:
 * Implementação do problema produtor/consumidor usando threads e variáveis de condição POSIX
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <pthread.h>
 #include <unistd.h>
 #include <time.h>
 
 #define BUFFER_SIZE 5  // Tamanho do buffer compartilhado
 #define NUM_ITEMS 20   // Número total de itens a serem produzidos
 #define NUM_PRODUCERS 2 // Número de produtores
 #define NUM_CONSUMERS 3 // Número de consumidores
 
 // Buffer circular compartilhado
 typedef struct {
     int items[BUFFER_SIZE];
     int in;     // Índice para inserção (produtor)
     int out;    // Índice para remoção (consumidor)
     int count;  // Número atual de itens no buffer
 } Buffer;
 
 Buffer buffer;
 
 // Mutex e variáveis de condição
 pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Controla o acesso ao buffer
 pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER; // Sinaliza itens disponíveis
 pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;  // Sinaliza vagas disponíveis
 
 // Contadores
 int items_produced = 0;  // Total de itens produzidos
 int items_consumed = 0;  // Total de itens consumidos
 
 // Inicializa o buffer
 void init_buffer() {
     buffer.in = 0;
     buffer.out = 0;
     buffer.count = 0;
 }
 
 // Produtor insere um item no buffer
 void produce(int item) {
     pthread_mutex_lock(&mutex);  // Obtém acesso exclusivo ao buffer
     
     // Espera enquanto o buffer estiver cheio
     while (buffer.count == BUFFER_SIZE) {
         printf("Buffer cheio. Produtor aguardando...\n");
         pthread_cond_wait(&not_full, &mutex);
     }
     
     // Insere o item no buffer
     buffer.items[buffer.in] = item;
     printf("Produtor: inseriu item %d na posição %d\n", item, buffer.in);
     buffer.in = (buffer.in + 1) % BUFFER_SIZE;
     buffer.count++;
     
     // Sinaliza que há um novo item disponível
     pthread_cond_signal(&not_empty);
     
     pthread_mutex_unlock(&mutex);  // Libera o acesso ao buffer
 }
 
 // Consumidor remove um item do buffer
 int consume() {
     int item;
     
     pthread_mutex_lock(&mutex);  // Obtém acesso exclusivo ao buffer
     
     // Espera enquanto o buffer estiver vazio
     while (buffer.count == 0) {
         printf("Buffer vazio. Consumidor aguardando...\n");
         pthread_cond_wait(&not_empty, &mutex);
     }
     
     // Remove o item do buffer
     item = buffer.items[buffer.out];
     printf("Consumidor: removeu item %d da posição %d\n", item, buffer.out);
     buffer.out = (buffer.out + 1) % BUFFER_SIZE;
     buffer.count--;
     
     // Sinaliza que há uma nova vaga disponível
     pthread_cond_signal(&not_full);
     
     pthread_mutex_unlock(&mutex);  // Libera o acesso ao buffer
     
     return item;
 }
 
 // Função executada pela thread produtora
 void* producer(void* arg) {
     int producer_id = *((int*)arg);
     
     while (1) {
         // Verifica se já produzimos todos os itens
         pthread_mutex_lock(&mutex);
         if (items_produced >= NUM_ITEMS) {
             pthread_mutex_unlock(&mutex);
             break;
         }
         
         items_produced++;
         int item = items_produced;
         pthread_mutex_unlock(&mutex);
         
         // Simula o tempo para produzir um item
         sleep(rand() % 3);
         
         printf("Produtor %d: produziu item %d\n", producer_id, item);
         
         // Insere o item no buffer
         produce(item);
     }
     
     printf("Produtor %d: terminou\n", producer_id);
     return NULL;
 }
 
 // Função executada pela thread consumidora
 void* consumer(void* arg) {
     int consumer_id = *((int*)arg);
     
     while (1) {
         // Verifica se já consumimos todos os itens
         pthread_mutex_lock(&mutex);
         if (items_consumed >= NUM_ITEMS) {
             pthread_mutex_unlock(&mutex);
             break;
         }
         items_consumed++;
         pthread_mutex_unlock(&mutex);
         
         // Remove um item do buffer
         int item = consume();
         
         // Simula o tempo para consumir um item
         sleep(rand() % 5);
         
         printf("Consumidor %d: consumiu item %d\n", consumer_id, item);
     }
     
     printf("Consumidor %d: terminou\n", consumer_id);
     return NULL;
 }
 
 int main() {
     // Inicializar semente para números aleatórios
     srand(time(NULL));
     
     // Inicializar o buffer
     init_buffer();
     
     // Criar threads para produtores e consumidores
     pthread_t producers[NUM_PRODUCERS];
     pthread_t consumers[NUM_CONSUMERS];
     int producer_ids[NUM_PRODUCERS];
     int consumer_ids[NUM_CONSUMERS];
     
     // Criar threads produtoras
     for (int i = 0; i < NUM_PRODUCERS; i++) {
         producer_ids[i] = i + 1;
         if (pthread_create(&producers[i], NULL, producer, &producer_ids[i]) != 0) {
             perror("Erro ao criar thread produtora");
             return 1;
         }
     }
     
     // Criar threads consumidoras
     for (int i = 0; i < NUM_CONSUMERS; i++) {
         consumer_ids[i] = i + 1;
         if (pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]) != 0) {
             perror("Erro ao criar thread consumidora");
             return 1;
         }
     }
     
     // Aguardar o término das threads produtoras
     for (int i = 0; i < NUM_PRODUCERS; i++) {
         pthread_join(producers[i], NULL);
     }
     
     // Aguardar o término das threads consumidoras
     for (int i = 0; i < NUM_CONSUMERS; i++) {
         pthread_join(consumers[i], NULL);
     }
     
     // Destruir o mutex e as variáveis de condição
     pthread_mutex_destroy(&mutex);
     pthread_cond_destroy(&not_empty);
     pthread_cond_destroy(&not_full);
     
     printf("Todos os itens foram produzidos e consumidos.\n");
     
     return 0;
 }