
 #include <stdio.h>
 #include <stdlib.h>
 #include <pthread.h>
 #include <semaphore.h>
 #include <unistd.h>
 
 // Estrutura que representa uma barreira
 typedef struct barrier_t {
     int n;                 // Número total de processos
     int count;             // Contador de processos que chegaram à barreira
     int generation;        // Geração da barreira (para distinguir entre barreiras sucessivas)
     pthread_mutex_t mutex; // Mutex para proteger o contador
     sem_t *sem;            // Array de semáforos, um para cada processo
 } barrier_t;
 
 // Inicialização de barreira para N processos
 void barrier_init(barrier_t *barrier, int N) {
     barrier->n = N;
     barrier->count = 0;
     barrier->generation = 0;
     
     // Inicializa o mutex
     pthread_mutex_init(&barrier->mutex, NULL);
     
     // Aloca e inicializa os semáforos para cada processo
     barrier->sem = (sem_t*)malloc(N * sizeof(sem_t));
     for (int i = 0; i < N; i++) {
         sem_init(&barrier->sem[i], 0, 0);
     }
 }
 
 // Operador de espera na barreira
 void barrier_wait(barrier_t *barrier, int process_id) {
     pthread_mutex_lock(&barrier->mutex);
     
     // Obtém a geração atual da barreira
     int my_generation = barrier->generation;
     
     // Incrementa o contador de processos que chegaram à barreira
     barrier->count++;
     
     if (barrier->count == barrier->n) {
         // Este é o último processo a chegar
         // Reseta o contador para a próxima barreira
         barrier->count = 0;
         
         // Incrementa a geração para evitar o problema de ultrapassagem
         barrier->generation++;
         
         // Libera todos os processos
         for (int i = 0; i < barrier->n; i++) {
             sem_post(&barrier->sem[i]);
         }
         
         pthread_mutex_unlock(&barrier->mutex);
     } else {
         // Não é o último processo, deve esperar
         pthread_mutex_unlock(&barrier->mutex);
         
         // Espera no semáforo correspondente ao seu ID
         sem_wait(&barrier->sem[process_id]);
         
         // Após ser liberado, verifica se a barreira foi reiniciada
         pthread_mutex_lock(&barrier->mutex);
         
         // Se a geração mudou, propagar o sinal para o próximo
         if (my_generation != barrier->generation) {
             sem_post(&barrier->sem[(process_id + 1) % barrier->n]);
         }
         
         pthread_mutex_unlock(&barrier->mutex);
     }
 }
 
 // Libera recursos da barreira
 void barrier_destroy(barrier_t *barrier) {
     for (int i = 0; i < barrier->n; i++) {
         sem_destroy(&barrier->sem[i]);
     }
     free(barrier->sem);
     pthread_mutex_destroy(&barrier->mutex);
 }
 
 // Funções para simular tarefas dos processos
 void task_1(int id) {
     printf("Processo %d executando tarefa 1\n", id);
     sleep(id + 1);  // Tempo de execução variável para demonstrar a sincronização
 }
 
 void task_2(int id) {
     printf("Processo %d executando tarefa 2\n", id);
     sleep((id % 3) + 1);
 }
 
 void task_3(int id) {
     printf("Processo %d executando tarefa 3\n", id);
     sleep((id % 2) + 1);
 }
 
 // Estrutura para passar dados para a thread
 typedef struct {
     int process_id;
     barrier_t *barrier;
 } ProcessData;
 
 // Função para os processos
 void* process(void* arg) {
     ProcessData *data = (ProcessData*)arg;
     int process_id = data->process_id;
     barrier_t *barrier = data->barrier;
     
     printf("Processo %d iniciado\n", process_id);
     
     // Primeira fase
     task_1(process_id);
     printf("Processo %d aguardando na primeira barreira\n", process_id);
     barrier_wait(barrier, process_id);
     printf("Processo %d passou da primeira barreira\n", process_id);
     
     // Segunda fase
     task_2(process_id);
     printf("Processo %d aguardando na segunda barreira\n", process_id);
     barrier_wait(barrier, process_id);
     printf("Processo %d passou da segunda barreira\n", process_id);
     
     // Terceira fase
     task_3(process_id);
     printf("Processo %d terminou\n", process_id);
     
     return NULL;
 }
 
 int main() {
     int N = 4;  // Número de processos
     pthread_t threads[N];
     ProcessData thread_data[N];
     
     // Criar e inicializar a barreira
     barrier_t barrier;
     barrier_init(&barrier, N);
     
     // Criar as threads para os processos
     for (int i = 0; i < N; i++) {
         thread_data[i].process_id = i;
         thread_data[i].barrier = &barrier;
         
         if (pthread_create(&threads[i], NULL, process, &thread_data[i]) != 0) {
             perror("Erro ao criar thread");
             return 1;
         }
     }
     
     // Aguardar o término das threads
     for (int i = 0; i < N; i++) {
         pthread_join(threads[i], NULL);
     }
     
     // Destruir a barreira
     barrier_destroy(&barrier);
     
     return 0;
 }