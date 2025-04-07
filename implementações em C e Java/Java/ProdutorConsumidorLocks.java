import java.util.LinkedList;
import java.util.Queue;
import java.util.Random;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;


public class ProdutorConsumidorLocks {
    private static final int BUFFER_SIZE = 5;      // Tamanho do buffer compartilhado
    private static final int NUM_ITEMS = 20;       // Número total de itens a serem produzidos
    private static final int NUM_PRODUCERS = 2;    // Número de produtores
    private static final int NUM_CONSUMERS = 3;    // Número de consumidores
    
    // Buffer compartilhado
    private static final Queue<Integer> buffer = new LinkedList<>();
    
    // Lock e variáveis de condição
    private static final Lock lock = new ReentrantLock();
    private static final Condition notEmpty = lock.newCondition();  // Sinaliza quando o buffer não está vazio
    private static final Condition notFull = lock.newCondition();   // Sinaliza quando o buffer não está cheio
    
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
                    lock.lock();
                    try {
                        // Verifica se já produzimos todos os itens
                        if (itemsProduced >= NUM_ITEMS) {
                            break;
                        }
                        
                        // Espera se o buffer estiver cheio
                        while (buffer.size() >= BUFFER_SIZE) {
                            System.out.println("Buffer cheio. Produtor " + id + " aguardando...");
                            notFull.await();
                        }
                        
                        // Produz um item
                        itemsProduced++;
                        int item = itemsProduced;
                        
                        // Simula o tempo para produzir um item (fora do lock)
                    } finally {
                        lock.unlock();
                    }
                    
                    // Simulação da produção (fora da região crítica)
                    Thread.sleep(random.nextInt(1000));
                    System.out.println("Produtor " + id + ": produziu item " + itemsProduced);
                    
                    // Insere o item no buffer
                    lock.lock();
                    try {
                        // Insere o item no buffer
                        buffer.add(itemsProduced);
                        System.out.println("Produtor " + id + ": inseriu item " + itemsProduced + " no buffer. Tamanho: " + buffer.size());
                        
                        // Notifica os consumidores que o buffer não está mais vazio
                        notEmpty.signal();
                    } finally {
                        lock.unlock();
                    }
                }
                
                System.out.println("Produtor " + id + ": terminou");
                
                // Sinaliza para todos os consumidores verificarem se há mais itens a consumir
                lock.lock();
                try {
                    notEmpty.signalAll();
                } finally {
                    lock.unlock();
                }
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
                    int item;
                    
                    lock.lock();
                    try {
                        // Verifica se já consumimos todos os itens e não há mais nada no buffer
                        if (itemsConsumed >= NUM_ITEMS && buffer.isEmpty()) {
                            break;
                        }
                        
                        // Espera se o buffer estiver vazio
                        while (buffer.isEmpty()) {
                            // Se já produzimos todos os itens e o buffer está vazio, termina
                            if (itemsProduced >= NUM_ITEMS) {
                                lock.unlock();
                                return;
                            }
                            
                            System.out.println("Buffer vazio. Consumidor " + id + " aguardando...");
                            notEmpty.await();
                            
                            // Após o await, precisamos verificar novamente se ainda há itens para consumir
                            if (itemsConsumed >= NUM_ITEMS && buffer.isEmpty()) {
                                break;
                            }
                        }
                        
                        // Remove o item do buffer
                        item = buffer.poll();
                        itemsConsumed++;
                        System.out.println("Consumidor " + id + ": removeu item " + item + " do buffer. Tamanho: " + buffer.size());
                        
                        // Notifica os produtores que o buffer não está mais cheio
                        notFull.signal();
                    } finally {
                        lock.unlock();
                    }
                    
                    // Simulação do consumo (fora da região crítica)
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