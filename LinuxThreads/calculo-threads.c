#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#define VECTOR_SIZE 100000000 // 100 milhões

// Estrutura para passar dados para a thread
typedef struct {
    double *x;         // Ponteiro para o vetor de dados
    double *results;   // Ponteiro para o vetor de resultados parciais
    int start_index;   // Índice inicial para esta thread
    int end_index;     // Índice final para esta thread
} ThreadData;

// Função que calcula f(x) = 2^(-2 * ((x-0.1)/0.9)^2) * (sin(5πx))^6
double calculate_f(double x) {
    double term1 = pow(2, -2 * pow((x - 0.1) / 0.9, 2));
    double term2 = pow(sin(5 * M_PI * x), 6);
    return term1 * term2;
}

// Função executada por cada thread
void* process_chunk(void *arg) {
    ThreadData *data = (ThreadData*) arg;
    double sum = 0.0;
    
    // Calcular f(x) para cada elemento no intervalo desta thread
    for (int i = data->start_index; i < data->end_index; i++) {
        sum += calculate_f(data->x[i]);
    }
    
    // Armazenar o resultado parcial
    *(data->results) = sum;
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <num_threads>\n", argv[0]);
        return 1;
    }
    
    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        printf("Número de threads deve ser maior que zero\n");
        return 1;
    }
    
    // Alocar memória para o vetor x
    double *x = (double*) malloc(VECTOR_SIZE * sizeof(double));
    if (!x) {
        perror("Falha ao alocar memória para o vetor");
        return 1;
    }
    
    // Inicializar o gerador de números aleatórios
    srand(time(NULL));
    
    // Preencher o vetor com valores aleatórios entre 0 e 1
    for (int i = 0; i < VECTOR_SIZE; i++) {
        x[i] = (double)rand() / RAND_MAX;
    }
    
    // Criar threads
    pthread_t *threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
    ThreadData *thread_data = (ThreadData*) malloc(num_threads * sizeof(ThreadData));
    double *partial_results = (double*) malloc(num_threads * sizeof(double));
    
    if (!threads || !thread_data || !partial_results) {
        perror("Falha ao alocar memória para as threads");
        free(x);
        free(threads);
        free(thread_data);
        free(partial_results);
        return 1;
    }
    
    // Calcular o tamanho do chunk para cada thread
    int chunk_size = VECTOR_SIZE / num_threads;
    
    // Iniciar a medição do tempo
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    // Criar e iniciar as threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].x = x;
        thread_data[i].results = &partial_results[i];
        thread_data[i].start_index = i * chunk_size;
        
        // A última thread pega o restante caso a divisão não seja exata
        if (i == num_threads - 1) {
            thread_data[i].end_index = VECTOR_SIZE;
        } else {
            thread_data[i].end_index = (i + 1) * chunk_size;
        }
        
        if (pthread_create(&threads[i], NULL, process_chunk, &thread_data[i]) != 0) {
            perror("Erro ao criar thread");
            free(x);
            free(threads);
            free(thread_data);
            free(partial_results);
            return 1;
        }
    }
    
    // Esperar que todas as threads terminem
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Somar os resultados parciais
    double total_sum = 0.0;
    for (int i = 0; i < num_threads; i++) {
        total_sum += partial_results[i];
    }
    
    // Calcular a média
    double average = total_sum / VECTOR_SIZE;
    
    // Finalizar a medição do tempo
    gettimeofday(&end_time, NULL);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + 
                          (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    
    // Exibir resultados
    printf("Valor médio de f(x): %f\n", average);
    printf("Tempo de execução: %.4f segundos\n", elapsed_time);
    
    // Liberar memória
    free(x);
    free(threads);
    free(thread_data);
    free(partial_results);
    
    return 0;
}