#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define VECTOR_SIZE 100000000 // 100 milhões
#define PI 3.14159265358979323846

// Estrutura para passar dados para a thread
typedef struct {
    double *x;         // Ponteiro para o vetor de dados
    double *results;   // Ponteiro para o vetor de resultados parciais
    int start_index;   // Índice inicial para esta thread
    int end_index;     // Índice final para esta thread
    int thread_id;     // ID da thread para debugging
} ThreadData;

// Variável para controle de execução
volatile sig_atomic_t execucao_interrompida = 0;

// Manipulador de sinal para interrupção
void manipular_sinal(int sig) {
    execucao_interrompida = 1;
    printf("\nRecebido sinal de interrupção (%d). Aguardando término das threads...\n", sig);
}

// Função que calcula f(x) = 2^(-2 * ((x-0.1)/0.9)^2) * (sin(5πx))^6
double calculate_f(double x) {
    double term1 = pow(2, -2 * pow((x - 0.1) / 0.9, 2));
    double term2 = pow(sin(5 * PI * x), 6);
    return term1 * term2;
}

// Função executada por cada thread
void* process_chunk(void *arg) {
    ThreadData *data = (ThreadData*) arg;
    double sum = 0.0;
    int processed = 0;
    
    // Calcular f(x) para cada elemento no intervalo desta thread
    for (int i = data->start_index; i < data->end_index && !execucao_interrompida; i++) {
        sum += calculate_f(data->x[i]);
        processed++;
        
        // Relatório de progresso a cada 10 milhões de elementos (somente para a thread 0)
        if (data->thread_id == 0 && processed % 10000000 == 0) {
            printf("Thread %d: Processados %d elementos\n", data->thread_id, processed);
        }
    }
    
    // Armazenar o resultado parcial
    *(data->results) = sum;
    
    printf("Thread %d: Finalizada - processou %d elementos\n", 
           data->thread_id, processed);
    
    return NULL;
}

// Função para gerar valores aleatórios melhorada
void generate_random_values(double *array, int size) {
    // Usar um gerador de melhor qualidade que rand()
    unsigned int seed = time(NULL);
    
    for (int i = 0; i < size; i++) {
        // Algoritmo de congruência linear mais sofisticado que rand()
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        array[i] = (double)seed / (double)0x7fffffff;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        printf("Número de threads deve ser maior que zero\n");
        return EXIT_FAILURE;
    }
    
    // Configurar manipulador de sinais
    signal(SIGINT, manipular_sinal);
    
    printf("Alocando memória para o vetor de %d elementos (%.2f GB)...\n", 
           VECTOR_SIZE, (VECTOR_SIZE * sizeof(double)) / (1024.0 * 1024.0 * 1024.0));
    
    // Alocar memória para o vetor x
    double *x = (double*) malloc(VECTOR_SIZE * sizeof(double));
    if (!x) {
        perror("Falha ao alocar memória para o vetor");
        return EXIT_FAILURE;
    }
    
    printf("Gerando valores aleatórios...\n");
    generate_random_values(x, VECTOR_SIZE);
    
    // Criar threads
    pthread_t *threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
    ThreadData *thread_data = (ThreadData*) malloc(num_threads * sizeof(ThreadData));
    double *partial_results = (double*) calloc(num_threads, sizeof(double));
    
    if (!threads || !thread_data || !partial_results) {
        perror("Falha ao alocar memória para as threads");
        free(x);
        free(threads);
        free(thread_data);
        free(partial_results);
        return EXIT_FAILURE;
    }
    
    // Calcular o tamanho do chunk para cada thread
    int chunk_size = VECTOR_SIZE / num_threads;
    
    printf("Iniciando cálculo com %d threads (%d elementos por thread)...\n", 
           num_threads, chunk_size);
    
    // Iniciar a medição do tempo
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    // Criar e iniciar as threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].x = x;
        thread_data[i].results = &partial_results[i];
        thread_data[i].start_index = i * chunk_size;
        thread_data[i].thread_id = i;
        
        // A última thread pega o restante caso a divisão não seja exata
        if (i == num_threads - 1) {
            thread_data[i].end_index = VECTOR_SIZE;
        } else {
            thread_data[i].end_index = (i + 1) * chunk_size;
        }
        
        int err;
        if ((err = pthread_create(&threads[i], NULL, process_chunk, &thread_data[i])) != 0) {
            fprintf(stderr, "Erro ao criar thread %d: %s\n", i, strerror(err));
            
            // Sinalizar interrupção para threads já criadas
            execucao_interrompida = 1;
            
            // Esperar threads já criadas
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            
            free(x);
            free(threads);
            free(thread_data);
            free(partial_results);
            return EXIT_FAILURE;
        }
    }
    
    // Esperar que todas as threads terminem
    for (int i = 0; i < num_threads; i++) {
        int err;
        if ((err = pthread_join(threads[i], NULL)) != 0) {
            fprintf(stderr, "Erro ao aguardar thread %d: %s\n", i, strerror(err));
        }
    }
    
    // Verificar se houve interrupção
    if (execucao_interrompida) {
        printf("Cálculo interrompido pelo usuário. Resultados parciais:\n");
    }
    
    // Somar os resultados parciais
    double total_sum = 0.0;
    for (int i = 0; i < num_threads; i++) {
        total_sum += partial_results[i];
        printf("Thread %d: Resultado parcial = %f\n", i, partial_results[i]);
    }
    
    // Calcular a média
    double average = total_sum / VECTOR_SIZE;
    
    // Finalizar a medição do tempo
    gettimeofday(&end_time, NULL);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + 
                          (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    
    // Exibir resultados
    printf("\nResultados finais:\n");
    printf("Valor médio de f(x): %.10f\n", average);
    printf("Soma total: %.10f\n", total_sum);
    printf("Tempo de execução: %.4f segundos\n", elapsed_time);
    printf("Velocidade: %.2f milhões de elementos/segundo\n", 
           VECTOR_SIZE / (elapsed_time * 1000000));
    
    // Liberar memória
    free(x);
    free(threads);
    free(thread_data);
    free(partial_results);
    
    return EXIT_SUCCESS;
}