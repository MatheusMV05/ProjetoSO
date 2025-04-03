import java.util.LinkedList;
import java.util.Queue;

class Buffer {
    private Queue<Integer> queue;
    private int capacity;

    public Buffer(int capacity) {
        this.capacity = capacity;
        queue = new LinkedList<>();
    }

    public synchronized void put(int item) throws InterruptedException {
        while (queue.size() == capacity)
            wait();
        queue.add(item);
        notifyAll();
    }

    public synchronized int get() throws InterruptedException {
        while (queue.isEmpty())
            wait();
        int item = queue.poll();
        notifyAll();
        return item;
    }
}

public class ProducerConsumerTwoBuffers {
    static final int NITEMS = 20;
    static Buffer buffer1 = new Buffer(5);
    static Buffer buffer2 = new Buffer(5);

    public static void main(String[] args) {
        Thread producer = new Thread(() -> {
            try {
                for (int i = 0; i < NITEMS; i++) {
                    System.out.println("Produtor produziu: " + i);
                    buffer1.put(i);
                    Thread.sleep(500);
                }
            } catch (InterruptedException e) { }
        });

        Thread intermediate = new Thread(() -> {
            try {
                for (int i = 0; i < NITEMS; i++) {
                    int item = buffer1.get();
                    item *= 2; // processamento
                    System.out.println("Intermediário colocou no buffer2: " + item);
                    buffer2.put(item);
                    Thread.sleep(500);
                }
            } catch (InterruptedException e) { }
        });

        Thread consumer = new Thread(() -> {
            try {
                for (int i = 0; i < NITEMS; i++) {
                    int item = buffer2.get();
                    System.out.println("Consumidor consumiu: " + item);
                    Thread.sleep(500);
                }
            } catch (InterruptedException e) { }
        });

        producer.start();
        intermediate.start();
        consumer.start();
    }
}
