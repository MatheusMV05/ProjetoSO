

 #include <stdio.h>
 #include <stdlib.h>
 #include <pthread.h>
 #include <semaphore.h>
 #include <unistd.h>
 #include <time.h>
 
 #define NUM_READERS 5    // Número de leitores
 #define NUM_WRITERS 2    // Número de escritores
 #define NUM_READS 10     // Número de leituras por leitor
 #define NUM_WRITES 5     // Número de escritas por escritor
 
 // Variáveis compartilhadas
 int shared_data = 0;        // Dado compartilhado
 int waiting_writers = 0;    // Número de escritores aguardando
 int active_readers = 0;     // Número de leitores ativos
 int active_writer = 0;      // Indica se há um escritor ativo (0 ou 1)
 
 // Semáforos e mutex
 sem_t mutex;                // Controla o acesso às variáveis compartilhadas
 sem_t write_mutex;          // Controla o acesso exclusivo para escrita
 sem_t reader_entry;         // Controla o acesso dos leitores à área compartilhada
 
 // Função de leitura
 void read_data(int reader_id) {
     // Solicita permissão para entrar na área compartilhada
     sem_wait(&reader_entry);
     
     // Adquire mutex para atualizar contador de leitores
     sem_wait(&mutex);
     
     // Incrementa o contador de leitores ativos
     active_readers++;
     
     // Se este é o primeiro leitor, bloqueia o mutex de escrita
     if (active_readers == 1) {
         sem_wait(&write_mutex);
     }
     
     // Libera o mutex
     sem_post(&mutex);
     
     // Permite que outros leitores entrem
     sem_post(&reader_entry);
     
     // Lê o dado compartilhado
     printf("Leitor %d: leu valor %d\n", reader_id, shared_data);
     
     // Simula o tempo de leitura
     usleep((rand() % 500) * 1000);  // De 0 a 500 ms
     
     // Adquire mutex para atualizar contador de leitores
     sem_wait(&mutex);
     
     // Decrementa o contador de leitores ativos
     active_readers--;
     
     // Se este é o último leitor, libera o mutex de escrita
     if (active_readers == 0) {
         sem_post(&write_mutex);
     }
     
     // Libera o mutex
     sem_post(&mutex);
 }
 
 // Função de escrita
 void write_data(int writer_id) {
     // Adquire mutex para atualizar contador de escritores
     sem_wait(&mutex);
     
     // Incrementa o contador de escritores aguardando
     waiting_writers++;
     
     // Se este é o primeiro escritor aguardando, bloqueia novos leitores
     if (waiting_writers == 1) {
         sem_wait(&reader_entry);
     }
     
     // Libera o mutex
     sem_post(&mutex);
     
     // Solicita acesso exclusivo à área compartilhada
     sem_wait(&write_mutex);
     
     // Indica que há um escritor ativo
     active_writer = 1;
     
     // Modifica o dado compartilhado
     shared_data++;
     printf("Escritor %d: escreveu valor %d\n", writer_id, shared_data);
     
     // Simula o tempo de escrita
     usleep((rand() % 1000) * 1000);  // De 0 a 1000 ms
     
     // Finaliza a escrita
     active_writer = 0;
     
     // Libera o acesso exclusivo
     sem_post(&write_mutex);
     
     // Adquire mutex para atualizar contador de escritores
     sem_wait(&mutex);
     
     // Decrementa o contador de escritores aguardando
     waiting_writers--;
     
     // Se não há mais escritores aguardando, libera os leitores
     if (waiting_writers == 0) {
         sem_post(&reader_entry);
     }
     
     // Libera o mutex
     sem_post(&mutex);
 }
 
 // Função executada pela thread leitora
 void* reader(void* arg) {
     int reader_id = *((int*)arg);
     
     printf("Leitor %d: iniciado\n", reader_id);
     
     // Realiza várias leituras
     for (int i = 0; i < NUM_READS; i++) {
         // Simula tempo entre leituras
         usleep((rand() % 300) * 1000);  // De 0 a 300 ms
         
         read_data(reader_id);
     }
     
     printf("Leitor %d: terminou\n", reader_id);
     return NULL;
 }
 
 // Função executada pela thread escritora
 void* writer(void* arg) {
     int writer_id = *((int*)arg);
     
     printf("Escritor %d: iniciado\n", writer_id);
     
     // Realiza várias escritas
     for (int i = 0; i < NUM_WRITES; i++) {
         // Simula tempo entre escritas
         usleep((rand() % 1000) * 1000);  // De 0 a 1000 ms
         
         write_data(writer_id);
     }
     
     printf("Escritor %d: terminou\n", writer_id);
     return NULL;
 }
 
 int main() {
     // Inicializar semente para números aleatórios
     srand(time(NULL));
     
     // Inicializar semáforos
     sem_init(&mutex, 0, 1);
     sem_init(&write_mutex, 0, 1);
     sem_init(&reader_entry, 0, 1);
     
     // Criar threads para leitores e escritores
     pthread_t readers[NUM_READERS];
     pthread_t writers[NUM_WRITERS];
     int reader_ids[NUM_READERS];
     int writer_ids[NUM_WRITERS];
     
     // Criar threads escritoras (primeiro para dar prioridade)
     for (int i = 0; i < NUM_WRITERS; i++) {
         writer_ids[i] = i + 1;
         if (pthread_create(&writers[i], NULL, writer, &writer_ids[i]) != 0) {
             perror("Erro ao criar thread escritora");
             return 1;
         }
     }
     
     // Criar threads leitoras
     for (int i = 0; i < NUM_READERS; i++) {
         reader_ids[i] = i + 1;
         if (pthread_create(&readers[i], NULL, reader, &reader_ids[i]) != 0) {
             perror("Erro ao criar thread leitora");
             return 1;
         }
     }
     
     // Aguardar o término das threads escritoras
     for (int i = 0; i < NUM_WRITERS; i++) {
         pthread_join(writers[i], NULL);
     }
     
     // Aguardar o término das threads leitoras
     for (int i = 0; i < NUM_READERS; i++) {
         pthread_join(readers[i], NULL);
     }
     
     // Destruir semáforos
     sem_destroy(&mutex);
     sem_destroy(&write_mutex);
     sem_destroy(&reader_entry);
     
     printf("Valor final do dado compartilhado: %d\n", shared_data);
     printf("Todas as operações de leitura e escrita foram concluídas.\n");
     
     return 0;
 }