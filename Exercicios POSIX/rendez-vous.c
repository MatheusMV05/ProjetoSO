/*
 * Solução para o Exercício 4:
 * Implementação do operador Rendez-Vous usando semáforos
 * 
 * O Rendez-Vous é um operador de sincronização forte entre dois processos,
 * onde um processo espera até que ambos cheguem ao ponto de encontro.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <pthread.h>
 #include <semaphore.h>
 #include <unistd.h>
 
 // Estrutura que representa um Rendez-Vous
 typedef struct rv_t {
     sem_t sem_a;      // Semáforo para o processo A
     sem_t sem_b;      // Semáforo para o processo B
     int count_a;      // Contador de chegadas do processo A
     int count_b;      // Contador de chegadas do processo B
     pthread_mutex_t mutex;  // Mutex para proteger os contadores
 } rv_t;
 
 // Inicialização do Rendez-Vous
 void rv_init(rv_t *rv) {
     // Inicializa os semáforos com valor 0 (bloqueados)
     sem_init(&rv->sem_a, 0, 0);
     sem_init(&rv->sem_b, 0, 0);
     
     // Inicializa os contadores
     rv->count_a = 0;
     rv->count_b = 0;
     
     // Inicializa o mutex
     pthread_mutex_init(&rv->mutex, NULL);
 }
 
 // Operador de espera no Rendez-Vous para o processo A
 void rv_wait_a(rv_t *rv) {
     pthread_mutex_lock(&rv->mutex);
     
     // Incrementa o contador de chegadas do processo A
     rv->count_a++;
     
     // Verifica se o processo B já chegou neste ponto
     if (rv->count_b < rv->count_a) {
         // B ainda não chegou, A deve esperar
         pthread_mutex_unlock(&rv->mutex);
         sem_wait(&rv->sem_a);
     } else {
         // B já chegou, libera B
         pthread_mutex_unlock(&rv->mutex);
         sem_post(&rv->sem_b);
     }
 }
 
 // Operador de espera no Rendez-Vous para o processo B
 void rv_wait_b(rv_t *rv) {
     pthread_mutex_lock(&rv->mutex);
     
     // Incrementa o contador de chegadas do processo B
     rv->count_b++;
     
     // Verifica se o processo A já chegou neste ponto
     if (rv->count_a < rv->count_b) {
         // A ainda não chegou, B deve esperar
         pthread_mutex_unlock(&rv->mutex);
         sem_wait(&rv->sem_b);
     } else {
         // A já chegou, libera A
         pthread_mutex_unlock(&rv->mutex);
         sem_post(&rv->sem_a);
     }
 }
 
 // Operador genérico de espera no Rendez-Vous
 void rv_wait(rv_t *rv, int process_id) {
     if (process_id == 0) {
         rv_wait_a(rv);
     } else {
         rv_wait_b(rv);
     }
 }
 
 // Função que simula as ações do processo A
 void A1() {
     printf("Executando A1\n");
     sleep(1);
 }
 
 void A2() {
     printf("Executando A2\n");
     sleep(1);
 }
 
 void A3() {
     printf("Executando A3\n");
     sleep(1);
 }
 
 // Função que simula as ações do processo B
 void B1() {
     printf("Executando B1\n");
     sleep(2);
 }
 
 void B2() {
     printf("Executando B2\n");
     sleep(2);
 }
 
 void B3() {
     printf("Executando B3\n");
     sleep(2);
 }
 
 // Função do processo A
 void* processo_A(void* arg) {
     rv_t* rv = (rv_t*)arg;
     
     A1();
     printf("Processo A chegou ao primeiro ponto de encontro\n");
     rv_wait(rv, 0);
     printf("Processo A passou do primeiro ponto de encontro\n");
     
     A2();
     printf("Processo A chegou ao segundo ponto de encontro\n");
     rv_wait(rv, 0);
     printf("Processo A passou do segundo ponto de encontro\n");
     
     A3();
     printf("Processo A terminou\n");
     
     return NULL;
 }
 
 // Função do processo B
 void* processo_B(void* arg) {
     rv_t* rv = (rv_t*)arg;
     
     B1();
     printf("Processo B chegou ao primeiro ponto de encontro\n");
     rv_wait(rv, 1);
     printf("Processo B passou do primeiro ponto de encontro\n");
     
     B2();
     printf("Processo B chegou ao segundo ponto de encontro\n");
     rv_wait(rv, 1);
     printf("Processo B passou do segundo ponto de encontro\n");
     
     B3();
     printf("Processo B terminou\n");
     
     return NULL;
 }
 
 int main() {
     pthread_t thread_A, thread_B;
     rv_t rv;
     
     // Inicializar o Rendez-Vous
     rv_init(&rv);
     
     // Criar as threads para os processos A e B
     pthread_create(&thread_A, NULL, processo_A, &rv);
     pthread_create(&thread_B, NULL, processo_B, &rv);
     
     // Aguardar o término das threads
     pthread_join(thread_A, NULL);
     pthread_join(thread_B, NULL);
     
     // Destruir o Rendez-Vous
     sem_destroy(&rv.sem_a);
     sem_destroy(&rv.sem_b);
     pthread_mutex_destroy(&rv.mutex);
     
     return 0;
 }