

 #include <stdio.h>
 #include <stdlib.h>
 #include <pthread.h>
 #include <unistd.h>
 #include <time.h>
 
 #define NUM_READERS 5    // Número de leitores
 #define NUM_WRITERS 2    // Número de escritores
 #define NUM_READS 10     // Número de leituras por leitor
 #define NUM_WRITES 5     // Número de escritas por escritor
 
 // Variáveis compartilhadas
 int shared_data = 0;              // Dado compartilhado
 int waiting_writers = 0;          // Número de escritores aguardando
 pthread_mutex_t writer_mutex;     // Mutex para controlar o acesso à fila de escritores
 pthread_mutex_t counter_mutex;    // Mutex para controlar o acesso aos contadores
 
 // RWLock com prioridade para escritores
 pthread_rwlock_t rwlock;
 pthread_rwlockattr_t rwlock_attr;
 
 // Função de leitura
 void read_data(int reader_id) {
     // Verifica se há escritores aguardando
     pthread_mutex_lock(&counter_mutex);
     int writers = waiting_writers;
     pthread_mutex_unlock(&counter_mutex);
     
     // Se houver escritores aguardando, cede a vez
     if (writers > 0) {
         usleep(10000);  // Atrasa um pouco para dar prioridade aos escritores
     }
     
     // Adquire a trava de leitura
     pthread_rwlock_rdlock(&rwlock);
     
     // Lê o dado compartilhado
     printf("Leitor %d: leu valor %d\n", reader_id, shared_data);
     
     // Simula o tempo de leitura
     usleep((rand() % 500) * 1000);  // De 0 a 500 ms
     
     // Libera a trava de leitura
     pthread_rwlock_unlock(&rwlock);
 }
 
 // Função de escrita
 void write_data(int writer_id) {
     // Indica que há um escritor aguardando
     pthread_mutex_lock(&counter_mutex);
     waiting_writers++;
     pthread_mutex_unlock(&counter_mutex);
     
     // Adquire a trava de escrita
     pthread_rwlock_wrlock(&rwlock);
     
     // Remove da fila de espera
     pthread_mutex_lock(&counter_mutex);
     waiting_writers--;
     pthread_mutex_unlock(&counter_mutex);
     
     // Modifica o dado compartilhado
     shared_data++;
     printf("Escritor %d: escreveu valor %d\n", writer_id, shared_data);
     
     // Simula o tempo de escrita
     usleep((rand() % 1000) * 1000);  // De 0 a 1000 ms
     
     // Libera a trava de escrita
     pthread_rwlock_unlock(&rwlock);
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
     
     // Inicializar mutex
     pthread_mutex_init(&writer_mutex, NULL);
     pthread_mutex_init(&counter_mutex, NULL);
     
     // Inicializar o atributo de rwlock para preferência de escritores
     pthread_rwlockattr_init(&rwlock_attr);
     
     
     // Inicializar o rwlock com os atributos definidos
     pthread_rwlock_init(&rwlock, &rwlock_attr);
     
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
     
     // Espera um pouco para garantir que os escritores têm prioridade
     usleep(100000);  // 100ms
     
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
     
     // Destruir o rwlock e seus atributos
     pthread_rwlock_destroy(&rwlock);
     pthread_rwlockattr_destroy(&rwlock_attr);
     pthread_mutex_destroy(&writer_mutex);
     pthread_mutex_destroy(&counter_mutex);
     
     printf("Valor final do dado compartilhado: %d\n", shared_data);
     printf("Todas as operações de leitura e escrita foram concluídas.\n");
     
     return 0;
 }
 
