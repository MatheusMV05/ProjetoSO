import java.util.Random;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;


public class LeitoresEscritoresRWLock {
    private static final int NUM_READERS = 5;   // Número de leitores
    private static final int NUM_WRITERS = 2;   // Número de escritores
    private static final int NUM_READS = 10;    // Número de leituras por leitor
    private static final int NUM_WRITES = 5;    // Número de escritas por escritor
    
    // Variável compartilhada
    private static int sharedData = 0;
    
    // Contador atômico para escritores esperando
    private static AtomicInteger waitingWriters = new AtomicInteger(0);
    
    // ReadWriteLock com preferência para escritor
    private static final ReadWriteLock rwLock = new ReentrantReadWriteLock(true);
    private static final Lock readLock = rwLock.readLock();
    private static final Lock writeLock = rwLock.writeLock();
    
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
                    
                    // Verifica se há escritores aguardando e dá uma chance para eles
                    if (waitingWriters.get() > 0) {
                        Thread.sleep(10);  // Pequeno atraso para dar prioridade aos escritores
                    }
                    
                    // Adquire a trava de leitura
                    readLock.lock();
                    
                    try {
                        // Lê o dado compartilhado
                        System.out.println("Leitor " + id + ": leu valor " + sharedData);
                        
                        // Simula o tempo de leitura
                        Thread.sleep(random.nextInt(500));
                    } finally {
                        // Libera a trava de leitura
                        readLock.unlock();
                    }
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
                    
                    // Indica que há um escritor aguardando
                    waitingWriters.incrementAndGet();
                    
                    // Adquire a trava de escrita
                    writeLock.lock();
                    
                    try {
                        // Remove da contagem de escritores aguardando
                        waitingWriters.decrementAndGet();
                        
                        // Modifica o dado compartilhado
                        sharedData++;
                        System.out.println("Escritor " + id + ": escreveu valor " + sharedData);
                        
                        // Simula o tempo de escrita
                        Thread.sleep(random.nextInt(1000));
                    } finally {
                        // Libera a trava de escrita
                        writeLock.unlock();
                    }
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
        
        // Pequeno atraso para garantir que os escritores comecem primeiro
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
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

