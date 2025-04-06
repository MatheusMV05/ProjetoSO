import java.util.concurrent.Semaphore;

public class Questao1_ProdutorConsumidor {
    static final int BUFFER_SIZE = 5;
    static int[] buffer = new int[BUFFER_SIZE];
    static int in = 0, out = 0;

    static Semaphore cheio = new Semaphore(0);
    static Semaphore vazio = new Semaphore(BUFFER_SIZE);
    static Semaphore mutex = new Semaphore(1);

    static class Produtor extends Thread {
        public void run() {
            try {
                for (int i = 0; i < 10; i++) {
                    int item = (int) (Math.random() * 100);
                    vazio.acquire();
                    mutex.acquire();

                    buffer[in] = item;
                    System.out.println("Produtor produziu: " + item + " na posição " + in);
                    in = (in + 1) % BUFFER_SIZE;

                    mutex.release();
                    cheio.release();

                    Thread.sleep(1000);
                }
            } catch (InterruptedException e) {}
        }
    }

    static class Consumidor extends Thread {
        public void run() {
            try {
                for (int i = 0; i < 10; i++) {
                    cheio.acquire();
                    mutex.acquire();

                    int item = buffer[out];
                    System.out.println("Consumidor consumiu: " + item + " da posição " + out);
                    out = (out + 1) % BUFFER_SIZE;

                    mutex.release();
                    vazio.release();

                    Thread.sleep(1500);
                }
            } catch (InterruptedException e) {}
        }
    }

    public static void main(String[] args) {
        new Produtor().start();
        new Consumidor().start();
    }
}
