#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

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

volatile sig_atomic_t continuar_execucao = 1;
int id_cliente = 0;
int *barbeiros_ocupados;  // Array para status dos barbeiros (0=dormindo, 1=ocupado)

// Parâmetros da simulação
int num_barbeiros;
int num_cadeiras;
int tempo_corte;
int intervalo_chegada;

// Manipulador de sinal para encerramento adequado
void manipular_sinal(int sig) {
    continuar_execucao = 0;
    printf("\nRecebido sinal de encerramento (%d). Finalizando...\n", sig);
}

// Inicializa a fila de clientes
int inicializar_fila(int capacidade) {
    fila.clientes = (int*) malloc(capacidade * sizeof(int));
    if (fila.clientes == NULL) {
        fprintf(stderr, "Erro ao alocar memória para a fila: %s\n", strerror(errno));
        return -1;
    }
    fila.capacidade = capacidade;
    fila.count = 0;
    fila.inicio = 0;
    fila.fim = 0;
    return 0;
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
        int err;
        
        // Bloqueia o mutex para acessar a fila
        if ((err = pthread_mutex_lock(&mutex)) != 0) {
            fprintf(stderr, "Erro ao bloquear mutex (barbeiro %d): %s\n", 
                    id_barbeiro, strerror(err));
            continue;
        }
        
        // Enquanto não houver clientes, o barbeiro dorme
        while (fila.count == 0 && continuar_execucao) {
            // Libera o mutex e bloqueia na condição
            if ((err = pthread_cond_wait(&cond_barbeiro, &mutex)) != 0) {
                fprintf(stderr, "Erro ao esperar pela condição (barbeiro %d): %s\n", 
                        id_barbeiro, strerror(err));
                break;
            }
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
        if ((err = pthread_cond_signal(&cond_cliente)) != 0) {
            fprintf(stderr, "Erro ao sinalizar condição (barbeiro %d): %s\n", 
                    id_barbeiro, strerror(err));
        }
        
        // Libera o mutex para deixar outros clientes entrarem enquanto corta o cabelo
        if ((err = pthread_mutex_unlock(&mutex)) != 0) {
            fprintf(stderr, "Erro ao desbloquear mutex (barbeiro %d): %s\n", 
                    id_barbeiro, strerror(err));
        }
        
        // Simula o tempo para cortar o cabelo
        sleep(tempo_corte);
        
        // Volta a bloquear o mutex para atualizar o status
        if ((err = pthread_mutex_lock(&mutex)) != 0) {
            fprintf(stderr, "Erro ao bloquear mutex (barbeiro %d): %s\n", 
                    id_barbeiro, strerror(err));
            continue;
        }
        
        printf("Barbeiro %d terminou de cortar o cabelo do cliente %d.\n", id_barbeiro, cliente);
        barbeiros_ocupados[id_barbeiro-1] = 0;  // Barbeiro disponível novamente
        
        if ((err = pthread_mutex_unlock(&mutex)) != 0) {
            fprintf(stderr, "Erro ao desbloquear mutex (barbeiro %d): %s\n", 
                    id_barbeiro, strerror(err));
        }
    }
    
    printf("Barbeiro %d encerrou o expediente.\n", id_barbeiro);
    return NULL;
}

// Função executada pela thread que gera os clientes
void* gerador_clientes(void *arg) {
    while (continuar_execucao) {
        // Cria um novo cliente
        int novo_cliente;
        int err;
        
        // Bloqueia o mutex para acessar o contador de clientes
        if ((err = pthread_mutex_lock(&mutex)) != 0) {
            fprintf(stderr, "Erro ao bloquear mutex (gerador): %s\n", strerror(err));
            sleep(1);
            continue;
        }
        
        id_cliente++;
        novo_cliente = id_cliente;
        
        if ((err = pthread_mutex_unlock(&mutex)) != 0) {
            fprintf(stderr, "Erro ao desbloquear mutex (gerador): %s\n", strerror(err));
        }
        
        printf("Cliente %d chegou.\n", novo_cliente);
        
        // Bloqueia o mutex para acessar a fila
        if ((err = pthread_mutex_lock(&mutex)) != 0) {
            fprintf(stderr, "Erro ao bloquear mutex (gerador): %s\n", strerror(err));
            sleep(1);
            continue;
        }
        
        // Tenta adicionar o cliente à fila
        if (enfileirar(novo_cliente)) {
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
                if ((err = pthread_cond_signal(&cond_barbeiro)) != 0) {
                    fprintf(stderr, "Erro ao sinalizar condição (gerador): %s\n", strerror(err));
                }
            }
        } else {
            // A fila está cheia, cliente vai embora
            printf("Cliente %d foi embora sem cortar o cabelo. Sala de espera cheia.\n", novo_cliente);
        }
        
        // Libera o mutex
        if ((err = pthread_mutex_unlock(&mutex)) != 0) {
            fprintf(stderr, "Erro ao desbloquear mutex (gerador): %s\n", strerror(err));
        }
        
        // Aguarda antes de gerar o próximo cliente
        sleep(intervalo_chegada);
    }
    
    return NULL;
}

void limpar_recursos() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_barbeiro);
    pthread_cond_destroy(&cond_cliente);
    
    if (fila.clientes != NULL) {
        free(fila.clientes);
        fila.clientes = NULL;
    }
    
    if (barbeiros_ocupados != NULL) {
        free(barbeiros_ocupados);
        barbeiros_ocupados = NULL;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Uso: %s <num_barbeiros> <num_cadeiras> <tempo_corte> <intervalo_chegada>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    // Parâmetros da simulação
    num_barbeiros = atoi(argv[1]);
    num_cadeiras = atoi(argv[2]);
    tempo_corte = atoi(argv[3]);
    intervalo_chegada = atoi(argv[4]);
    
    if (num_barbeiros <= 0 || num_cadeiras < 0 || tempo_corte <= 0 || intervalo_chegada <= 0) {
        printf("Todos os parâmetros devem ser maiores que zero\n");
        return EXIT_FAILURE;
    }
    
    // Configurar manipulador de sinais
    signal(SIGINT, manipular_sinal);
    
    // Inicializar a fila de clientes
    if (inicializar_fila(num_cadeiras) != 0) {
        return EXIT_FAILURE;
    }
    
    // Inicializar o array de status dos barbeiros
    barbeiros_ocupados = (int*) calloc(num_barbeiros, sizeof(int));
    if (barbeiros_ocupados == NULL) {
        fprintf(stderr, "Erro ao alocar memória para status dos barbeiros: %s\n", strerror(errno));
        limpar_recursos();
        return EXIT_FAILURE;
    }
    
    // Criar threads para os barbeiros
    pthread_t *barbeiros = (pthread_t*) malloc(num_barbeiros * sizeof(pthread_t));
    int *ids_barbeiros = (int*) malloc(num_barbeiros * sizeof(int));
    
    if (barbeiros == NULL || ids_barbeiros == NULL) {
        fprintf(stderr, "Erro ao alocar memória para threads: %s\n", strerror(errno));
        free(barbeiros);
        free(ids_barbeiros);
        limpar_recursos();
        return EXIT_FAILURE;
    }
    
    for (int i = 0; i < num_barbeiros; i++) {
        ids_barbeiros[i] = i + 1;  // IDs dos barbeiros começam em 1
        
        int err;
        if ((err = pthread_create(&barbeiros[i], NULL, barbeiro_func, &ids_barbeiros[i])) != 0) {
            fprintf(stderr, "Erro ao criar thread do barbeiro %d: %s\n", i+1, strerror(err));
            // Sinalizar para encerrar as threads já criadas
            continuar_execucao = 0;
            pthread_cond_broadcast(&cond_barbeiro);
            pthread_cond_broadcast(&cond_cliente);
            
            // Aguardar as threads que já foram criadas
            for (int j = 0; j < i; j++) {
                pthread_join(barbeiros[j], NULL);
            }
            
            free(barbeiros);
            free(ids_barbeiros);
            limpar_recursos();
            return EXIT_FAILURE;
        }
    }
    
    // Criar thread para o gerador de clientes
    pthread_t gerador;
    int err;
    if ((err = pthread_create(&gerador, NULL, gerador_clientes, NULL)) != 0) {
        fprintf(stderr, "Erro ao criar thread do gerador de clientes: %s\n", strerror(err));
        
        // Sinalizar para encerrar as threads
        continuar_execucao = 0;
        pthread_cond_broadcast(&cond_barbeiro);
        pthread_cond_broadcast(&cond_cliente);
        
        // Aguardar as threads dos barbeiros
        for (int i = 0; i < num_barbeiros; i++) {
            pthread_join(barbeiros[i], NULL);
        }
        
        free(barbeiros);
        free(ids_barbeiros);
        limpar_recursos();
        return EXIT_FAILURE;
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
    free(barbeiros);
    free(ids_barbeiros);
    limpar_recursos();
    
    printf("Simulação encerrada com sucesso.\n");
    return EXIT_SUCCESS;
}