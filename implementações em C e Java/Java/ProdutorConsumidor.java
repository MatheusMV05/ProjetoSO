import java.util.LinkedList;
import java.util.Queue;
import java.util.Random;
import java.util.concurrent.Semaphore;

/**
 * Implementação do problema produtor/consumidor usando threads e semáforos em Java
 */
public class ProdutorConsumidor {
    private static final int BUFFER_SIZE = 5;      // Tamanho do buffer compartilhado
    private static final int NUM_ITEMS = 20;       // Número total de itens a serem produzidos
    private static final int NUM_PRODUCERS = 2;    // Número de produtores
    private static final int NUM_CONSUMERS = 3;    // Número de consumidores
    
    // Buffer compartilhado
    private static final Queue<Integer> buffer = new LinkedList<>();
    
    // Semáforos e locks
    private static final Semaphore empty = new Semaphore(BUFFER_SIZE);  // Controla as vagas disponíveis
    private static final Semaphore full = new Semaphore(0);             // Controla os itens disponíveis
    private static final Semaphore mutex = new Semaphore(1);            // Controla o acesso ao buffer
    
    // Contadores
    private static int itemsProduced = 0;  // Total de itens produzidos
    private static int itemsConsumed = 0;  // Total de itens consumidos
    
    // Classe que implementa o produtor
    static class Producer implements Runnable {
        private final int id;
        private final Random random = new Random();
        
        public Producer(int id) {
            this.id = id;
        }
        
        @Override
        public void run() {
            try {
                while (true) {
                    // Verifica se já produzimos todos os itens
                    mutex.acquire();
                    if (itemsProduced >= NUM_ITEMS) {
                        mutex.release();
                        break;
                    }
                    
                    itemsProduced++;
                    int item = itemsProduced;
                    mutex.release();
                    
                    // Simula o tempo para produzir um item
                    Thread.sleep(random.nextInt(1000));
                    
                    System.out.println("Produtor " + id + ": produziu item " + item);
                    
                    // Insere o item no buffer
                    empty.acquire();       // Aguarda uma vaga no buffer
                    mutex.acquire();       // Obtém acesso exclusivo ao buffer
                    
                    // Insere o item no buffer
                    buffer.add(item);
                    System.out.println("Produtor " + id + ": inseriu item " + item + " no buffer. Tamanho: " + buffer.size());
                    
                    mutex.release();       // Libera o acesso ao buffer
                    full.release();        // Sinaliza que há um novo item disponível
                }
                
                System.out.println("Produtor " + id + ": terminou");
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }
    
    // Classe que implementa o consumidor
    static class Consumer implements Runnable {
        private final int id;
        private final Random random = new Random();
        
        public Consumer(int id) {
            this.id = id;
        }
        
        @Override
        public void run() {
            try {
                while (true) {
                    // Verifica se já consumimos todos os itens
                    mutex.acquire();
                    if (itemsConsumed >= NUM_ITEMS) {
                        mutex.release();
                        break;
                    }
                    itemsConsumed++;
                    mutex.release();
                    
                    // Remove um item do buffer
                    full.acquire();        // Aguarda um item disponível
                    mutex.acquire();       // Obtém acesso exclusivo ao buffer
                    
                    // Remove o item do buffer
                    int item = buffer.poll();
                    System.out.println("Consumidor " + id + ": removeu item " + item + " do buffer. Tamanho: " + buffer.size());
                    
                    mutex.release();       // Libera o acesso ao buffer
                    empty.release();       // Sinaliza que há uma nova vaga disponível
                    
                    // Simula o tempo para consumir um item
                    Thread.sleep(random.nextInt(2000));
                    
                    System.out.println("Consumidor " + id + ": consumiu item " + item);
                }
                
                System.out.println("Consumidor " + id + ": terminou");
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }
    
    public static void main(String[] args) {
        // Criar threads para produtores e consumidores
        Thread[] producers = new Thread[NUM_PRODUCERS];
        Thread[] consumers = new Thread[NUM_CONSUMERS];
        
        // Criar threads produtoras
        for (int i = 0; i < NUM_PRODUCERS; i++) {
            producers[i] = new Thread(new Producer(i + 1));
            producers[i].start();
        }
        
        // Criar threads consumidoras
        for (int i = 0; i < NUM_CONSUMERS; i++) {
            consumers[i] = new Thread(new Consumer(i + 1));
            consumers[i].start();
        }
        
        // Aguardar o término das threads produtoras
        for (int i = 0; i < NUM_PRODUCERS; i++) {
            try {
                producers[i].join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
        
        // Aguardar o término das threads consumidoras
        for (int i = 0; i < NUM_CONSUMERS; i++) {
            try {
                consumers[i].join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
        
        System.out.println("Todos os itens foram produzidos e consumidos.");
    }
}