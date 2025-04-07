import java.util.Random;
import java.util.concurrent.Semaphore;

/**
 * Implementação do problema leitores/escritores com priorização para escritores em Java
 */
public class LeitoresEscritores {
    private static final int NUM_READERS = 5;   // Número de leitores
    private static final int NUM_WRITERS = 2;   // Número de escritores
    private static final int NUM_READS = 10;    // Número de leituras por leitor
    private static final int NUM_WRITES = 5;    // Número de escritas por escritor
    
    // Variáveis compartilhadas
    private static int sharedData = 0;        // Dado compartilhado
    private static int waitingWriters = 0;    // Número de escritores aguardando
    private static int activeReaders = 0;     // Número de leitores ativos
    private static boolean activeWriter = false; // Indica se há um escritor ativo
    
    // Semáforos
    private static final Semaphore mutex = new Semaphore(1);           // Controla o acesso às variáveis compartilhadas
    private static final Semaphore writeMutex = new Semaphore(1);      // Controla o acesso exclusivo para escrita
    private static final Semaphore readerEntry = new Semaphore(1);     // Controla o acesso dos leitores à área compartilhada
    
    // Classe que implementa o leitor
    static class Reader implements Runnable {
        private final int id;
        private final Random random = new Random();
        
        public Reader(int id) {
            this.id = id;
        }
        
        @Override
        public void run() {
            try {
                System.out.println("Leitor " + id + ": iniciado");
                
                // Realiza várias leituras
                for (int i = 0; i < NUM_READS; i++) {
                    // Simula tempo entre leituras
                    Thread.sleep(random.nextInt(300));
                    
                    // Solicita permissão para entrar na área compartilhada
                    readerEntry.acquire();
                    
                    // Adquire mutex para atualizar contador de leitores
                    mutex.acquire();
                    
                    // Incrementa o contador de leitores ativos
                    activeReaders++;
                    
                    // Se este é o primeiro leitor, bloqueia o mutex de escrita
                    if (activeReaders == 1) {
                        writeMutex.acquire();
                    }
                    
                    // Libera o mutex
                    mutex.release();
                    
                    // Permite que outros leitores entrem
                    readerEntry.release();
                    
                    // Lê o dado compartilhado
                    System.out.println("Leitor " + id + ": leu valor " + sharedData);
                    
                    // Simula o tempo de leitura
                    Thread.sleep(random.nextInt(500));
                    
                    // Adquire mutex para atualizar contador de leitores
                    mutex.acquire();
                    
                    // Decrementa o contador de leitores ativos
                    activeReaders--;
                    
                    // Se este é o último leitor, libera o mutex de escrita
                    if (activeReaders == 0) {
                        writeMutex.release();
                    }
                    
                    // Libera o mutex
                    mutex.release();
                }
                
                System.out.println("Leitor " + id + ": terminou");
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }
    
    // Classe que implementa o escritor
    static class Writer implements Runnable {
        private final int id;
        private final Random random = new Random();
        
        public Writer(int id) {
            this.id = id;
        }
        
        @Override
        public void run() {
            try {
                System.out.println("Escritor " + id + ": iniciado");
                
                // Realiza várias escritas
                for (int i = 0; i < NUM_WRITES; i++) {
                    // Simula tempo entre escritas
                    Thread.sleep(random.nextInt(1000));
                    
                    // Adquire mutex para atualizar contador de escritores
                    mutex.acquire();
                    
                    // Incrementa o contador de escritores aguardando
                    waitingWriters++;
                    
                    // Se este é o primeiro escritor aguardando, bloqueia novos leitores
                    if (waitingWriters == 1) {
                        readerEntry.acquire();
                    }
                    
                    // Libera o mutex
                    mutex.release();
                    
                    // Solicita acesso exclusivo à área compartilhada
                    writeMutex.acquire();
                    
                    // Indica que há um escritor ativo
                    activeWriter = true;
                    
                    // Modifica o dado compartilhado
                    sharedData++;
                    System.out.println("Escritor " + id + ": escreveu valor " + sharedData);
                    
                    // Simula o tempo de escrita
                    Thread.sleep(random.nextInt(1000));
                    
                    // Finaliza a escrita
                    activeWriter = false;
                    
                    // Libera o acesso exclusivo
                    writeMutex.release();
                    
                    // Adquire mutex para atualizar contador de escritores
                    mutex.acquire();
                    
                    // Decrementa o contador de escritores aguardando
                    waitingWriters--;
                    
                    // Se não há mais escritores aguardando, libera os leitores
                    if (waitingWriters == 0) {
                        readerEntry.release();
                    }
                    
                    // Libera o mutex
                    mutex.release();
                }
                
                System.out.println("Escritor " + id + ": terminou");
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }
    
    public static void main(String[] args) {
        // Criar threads para leitores e escritores
        Thread[] readers = new Thread[NUM_READERS];
        Thread[] writers = new Thread[NUM_WRITERS];
        
        // Criar threads escritoras (primeiro para dar prioridade)
        for (int i = 0; i < NUM_WRITERS; i++) {
            writers[i] = new Thread(new Writer(i + 1));
            writers[i].start();
        }
        
        // Criar threads leitoras
        for (int i = 0; i < NUM_READERS; i++) {
            readers[i] = new Thread(new Reader(i + 1));
            readers[i].start();
        }
        
        // Aguardar o término das threads escritoras
        for (int i = 0; i < NUM_WRITERS; i++) {
            try {
                writers[i].join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
        
        // Aguardar o término das threads leitoras
        for (int i = 0; i < NUM_READERS; i++) {
            try {
                readers[i].join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
        
        System.out.println("Valor final do dado compartilhado: " + sharedData);
        System.out.println("Todas as operações de leitura e escrita foram concluídas.");
    }
}

/*
 * Observação sobre a prioridade para escritores:
 * Esta implementação garante a prioridade para escritores de duas formas:
 * 
 * 1. Quando um escritor está aguardando, ele bloqueia a entrada de novos leitores
 *    utilizando o semáforo readerEntry. Os leitores que já estão na área compartilhada
 *    podem concluir suas leituras, mas nenhum novo leitor poderá entrar.
 * 
 * 2. Os escritores são iniciados antes dos leitores, o que lhes dá uma vantagem inicial.
 * 
 * Esta estratégia garante que, quando há escritores aguardando, eles terão prioridade
 * sobre novos leitores, evitando assim a inanição dos escritores.
 */