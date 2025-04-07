/*
 * Solução para o Exercício 3:
 * Sincronização de robôs Bart, Lisa, Maggie na sequência
 * Bart → Lisa → Maggie → Lisa → Bart → Lisa → Maggie → ...
 * usando semáforos
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <pthread.h>
 #include <semaphore.h>
 #include <unistd.h>
 
 // Semáforos para sincronizar a ordem dos movimentos
 sem_t sem_bart;    // Controla quando Bart pode se mover
 sem_t sem_lisa;    // Controla quando Lisa pode se mover
 sem_t sem_maggie;  // Controla quando Maggie pode se mover
 
 // Contador para controlar a alternância de movimentos da Lisa
 int lisa_turn = 0;  // 0 = primeira vez, 1 = segunda vez
 
 // Simulação de movimento do robô
 void move(const char* robot_name) {
     printf("Robô %s está se movendo\n", robot_name);
     // Simula o tempo que o robô leva para se mover
     usleep(500000);  // 500ms
     printf("Robô %s terminou de se mover\n", robot_name);
 }
 
 // Função para o robô Bart
 void* robot_bart(void* arg) {
     while (1) {
         // Espera até ser a vez do Bart
         sem_wait(&sem_bart);
         
         // Bart se move
         move("Bart");
         
         // Agora é a vez da Lisa
         sem_post(&sem_lisa);
     }
     
     return NULL;
 }
 
 // Função para o robô Lisa
 void* robot_lisa(void* arg) {
     while (1) {
         // Espera até ser a vez da Lisa
         sem_wait(&sem_lisa);
         
         // Lisa se move
         move("Lisa");
         
         // Decide qual robô vai se mover em seguida
         if (lisa_turn == 0) {
             // Após o primeiro movimento da Lisa, é a vez da Maggie
             lisa_turn = 1;
             sem_post(&sem_maggie);
         } else {
             // Após o segundo movimento da Lisa, é a vez do Bart
             lisa_turn = 0;
             sem_post(&sem_bart);
         }
     }
     
     return NULL;
 }
 
 // Função para o robô Maggie
 void* robot_maggie(void* arg) {
     while (1) {
         // Espera até ser a vez da Maggie
         sem_wait(&sem_maggie);
         
         // Maggie se move
         move("Maggie");
         
         // Agora é a vez da Lisa novamente
         sem_post(&sem_lisa);
     }
     
     return NULL;
 }
 
 int main() {
     pthread_t thread_bart, thread_lisa, thread_maggie;
     
     // Inicializar os semáforos
     // Bart começa (valor 1), os outros esperam (valor 0)
     sem_init(&sem_bart, 0, 1);
     sem_init(&sem_lisa, 0, 0);
     sem_init(&sem_maggie, 0, 0);
     
     // Criar as threads para os robôs
     pthread_create(&thread_bart, NULL, robot_bart, NULL);
     pthread_create(&thread_lisa, NULL, robot_lisa, NULL);
     pthread_create(&thread_maggie, NULL, robot_maggie, NULL);
     
     // Aguardar um comando para encerrar a simulação
     printf("\nPressione Enter para encerrar a simulação...\n");
     getchar();
     
     // Não precisamos aguardar as threads terminarem, vamos encerrar o programa
     
     // Destruir os semáforos
     sem_destroy(&sem_bart);
     sem_destroy(&sem_lisa);
     sem_destroy(&sem_maggie);
     
     return 0;
 }