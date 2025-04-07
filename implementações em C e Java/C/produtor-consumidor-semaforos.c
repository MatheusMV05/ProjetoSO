

 #include <stdio.h>
 #include <stdlib.h>
 #include <pthread.h>
 #include <semaphore.h>
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
 } Buffer;
 
 Buffer buffer;
 
 // Semáforos e mutex
 sem_t empty;       // Controla as vagas disponíveis (inicialmente BUFFER_SIZE)
 sem_t full;        // Controla os itens disponíveis (inicialmente 0)
 pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Controla o acesso ao buffer
 
 // Contadores
 int items_produced = 0;  // Total de itens produzidos
 int items_consumed = 0;  // Total de itens consumidos
 
 // Inicializa o buffer
 void init_buffer() {
     buffer.in = 0;
     buffer.out = 0;
 }
 
 // Produtor insere um item no buffer
 void produce(int item) {
     sem_wait(&empty);          // Aguarda uma vaga no buffer
     pthread_mutex_lock(&mutex);  // Obtém acesso exclusivo ao buffer
     
     // Insere o item no buffer
     buffer.items[buffer.in] = item;
     printf("Produtor: inseriu item %d na posição %d\n", item, buffer.in);
     buffer.in = (buffer.in + 1) % BUFFER_SIZE;
     
     pthread_mutex_unlock(&mutex);  // Libera o acesso ao buffer
     sem_post(&full);            // Sinaliza que há um novo item disponível
 }
 
 // Consumidor remove um item do buffer
 int consume() {
     int item;
     
     sem_wait(&full);           // Aguarda um item disponível
     pthread_mutex_lock(&mutex);  // Obtém acesso exclusivo ao buffer
     
     // Remove o item do buffer
     item = buffer.items[buffer.out];
     printf("Consumidor: removeu item %d da posição %d\n", item, buffer.out);
     buffer.out = (buffer.out + 1) % BUFFER_SIZE;
     
     pthread_mutex_unlock(&mutex);  // Libera o acesso ao buffer
     sem_post(&empty);          // Sinaliza que há uma nova vaga disponível
     
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
     
     // Inicializar os semáforos
     sem_init(&empty, 0, BUFFER_SIZE);
     sem_init(&full, 0, 0);
     
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
     
     // Destruir os semáforos
     sem_destroy(&empty);
     sem_destroy(&full);
     pthread_mutex_destroy(&mutex);
     
     printf("Todos os itens foram produzidos e consumidos.\n");
     
     return 0;
 }