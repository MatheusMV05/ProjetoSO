#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Estrutura para representar a fila de clientes
typedef struct {
    int *clientes;     // Array para os IDs dos clientes
    int capacidade;    // Capacidade máxima da sala de espera
    int count;         // Número atual de clientes na fila
    int inicio;        // Índice do primeiro cliente na fila
    int fim;           // Índice onde o próximo cliente será inserido
} FilaClientes;

// Variáveis compartilhadas
FilaClientes fila;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_barbeiro = PTHREAD_COND_INITIALIZER;  // Condição para acordar o barbeiro
pthread_cond_t cond_cliente = PTHREAD_COND_INITIALIZER;   // Condição para o cliente esperar o barbeiro

int continuar_execucao = 1;
int id_cliente = 0;
int *barbeiros_ocupados;  // Array para status dos barbeiros (0=dormindo, 1=ocupado)

// Parâmetros da simulação
int num_barbeiros;
int num_cadeiras;
int tempo_corte;
int intervalo_chegada;

// Inicializa a fila de clientes
void inicializar_fila(int capacidade) {
    fila.clientes = (int*) malloc(capacidade * sizeof(int));
    fila.capacidade = capacidade;
    fila.count = 0;
    fila.inicio = 0;
    fila.fim = 0;
}

// Adiciona um cliente à fila, retorna 1 se sucesso, 0 se fila cheia
int enfileirar(int id_cliente) {
    if (fila.count >= fila.capacidade) {
        return 0;  // Fila cheia
    }
    
    fila.clientes[fila.fim] = id_cliente;
    fila.fim = (fila.fim + 1) % fila.capacidade;
    fila.count++;
    return 1;
}

// Remove um cliente da fila, retorna o ID do cliente ou -1 se fila vazia
int desenfileirar() {
    if (fila.count == 0) {
        return -1;  // Fila vazia
    }
    
    int cliente = fila.clientes[fila.inicio];
    fila.inicio = (fila.inicio + 1) % fila.capacidade;
    fila.count--;
    return cliente;
}

// Função executada pelas threads dos barbeiros
void* barbeiro_func(void *arg) {
    int id_barbeiro = *((int*) arg);
    
    printf("Barbeiro %d dormindo.\n", id_barbeiro);
    
    while (continuar_execucao) {
        // Bloqueia o mutex para acessar a fila
        pthread_mutex_lock(&mutex);
        
        // Enquanto não houver clientes, o barbeiro dorme
        while (fila.count == 0 && continuar_execucao) {
            // Libera o mutex e bloqueia na condição
            pthread_cond_wait(&cond_barbeiro, &mutex);
        }
        
        // Verificar se devemos encerrar
        if (!continuar_execucao) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        // Atender um cliente
        int cliente = desenfileirar();
        barbeiros_ocupados[id_barbeiro-1] = 1;  // Marcar barbeiro como ocupado
        
        printf("Barbeiro %d acordou.\n", id_barbeiro);
        printf("Barbeiro %d cortando o cabelo do cliente %d.\n", id_barbeiro, cliente);
        
        // Sinaliza que um cliente pode entrar se houver espaço na fila
        pthread_cond_signal(&cond_cliente);
        
        // Libera o mutex para deixar outros clientes entrarem enquanto corta o cabelo
        pthread_mutex_unlock(&mutex);
        
        // Simula o tempo para cortar o cabelo
        sleep(tempo_corte);
        
        // Volta a bloquear o mutex para atualizar o status
        pthread_mutex_lock(&mutex);
        
        printf("Barbeiro %d terminou de cortar o cabelo do cliente %d.\n", id_barbeiro, cliente);
        barbeiros_ocupados[id_barbeiro-1] = 0;  // Barbeiro disponível novamente
        
        pthread_mutex_unlock(&mutex);
    }
    
    printf("Barbeiro %d encerrou o expediente.\n", id_barbeiro);
    return NULL;
}

// Função executada pela thread que gera os clientes
void* gerador_clientes(void *arg) {
    while (continuar_execucao) {
        // Cria um novo cliente
        id_cliente++;
        printf("Cliente %d chegou.\n", id_cliente);
        
        // Bloqueia o mutex para acessar a fila
        pthread_mutex_lock(&mutex);
        
        // Tenta adicionar o cliente à fila
        if (enfileirar(id_cliente)) {
            // Verifica se algum barbeiro está dormindo e acorda
            int barbeiro_acordado = 0;
            for (int i = 0; i < num_barbeiros; i++) {
                if (barbeiros_ocupados[i] == 0) { // Barbeiro dormindo
                    barbeiro_acordado = 1;
                    break;
                }
            }
            
            if (barbeiro_acordado) {
                // Acorda um barbeiro
                pthread_cond_signal(&cond_barbeiro);
            }
        } else {
            // A fila está cheia, cliente vai embora
            printf("Cliente %d foi embora sem cortar o cabelo. Sala de espera cheia.\n", id_cliente);
        }
        
        // Libera o mutex
        pthread_mutex_unlock(&mutex);
        
        // Aguarda antes de gerar o próximo cliente
        sleep(intervalo_chegada);
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Uso: %s <num_barbeiros> <num_cadeiras> <tempo_corte> <intervalo_chegada>\n", argv[0]);
        return 1;
    }
    
    // Parâmetros da simulação
    num_barbeiros = atoi(argv[1]);
    num_cadeiras = atoi(argv[2]);
    tempo_corte = atoi(argv[3]);
    intervalo_chegada = atoi(argv[4]);
    
    if (num_barbeiros <= 0 || num_cadeiras < 0 || tempo_corte <= 0 || intervalo_chegada <= 0) {
        printf("Todos os parâmetros devem ser maiores que zero\n");
        return 1;
    }
    
    // Inicializar a fila de clientes
    inicializar_fila(num_cadeiras);
    
    // Inicializar o array de status dos barbeiros
    barbeiros_ocupados = (int*) calloc(num_barbeiros, sizeof(int));
    
    // Criar threads para os barbeiros
    pthread_t *barbeiros = (pthread_t*) malloc(num_barbeiros * sizeof(pthread_t));
    int *ids_barbeiros = (int*) malloc(num_barbeiros * sizeof(int));
    
    for (int i = 0; i < num_barbeiros; i++) {
        ids_barbeiros[i] = i + 1;  // IDs dos barbeiros começam em 1
        
        if (pthread_create(&barbeiros[i], NULL, barbeiro_func, &ids_barbeiros[i]) != 0) {
            perror("Erro ao criar thread do barbeiro");
            return 1;
        }
    }
    
    // Criar thread para o gerador de clientes
    pthread_t gerador;
    if (pthread_create(&gerador, NULL, gerador_clientes, NULL) != 0) {
        perror("Erro ao criar thread do gerador de clientes");
        return 1;
    }
    
    // Aguardar um comando para encerrar a simulação
    printf("\nPressione Enter para encerrar a simulação...\n");
    getchar();
    
    // Sinalizar para todas as threads terminarem
    pthread_mutex_lock(&mutex);
    continuar_execucao = 0;
    pthread_cond_broadcast(&cond_barbeiro);
    pthread_cond_broadcast(&cond_cliente);
    pthread_mutex_unlock(&mutex);
    
    // Aguardar o término da thread do gerador
    pthread_join(gerador, NULL);
    
    // Aguardar o término das threads dos barbeiros
    for (int i = 0; i < num_barbeiros; i++) {
        pthread_join(barbeiros[i], NULL);
    }
    
    // Liberar memória
    free(fila.clientes);
    free(barbeiros);
    free(ids_barbeiros);
    free(barbeiros_ocupados);
    
    return 0;
}