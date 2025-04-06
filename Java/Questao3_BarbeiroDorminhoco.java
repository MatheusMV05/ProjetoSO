import java.util.concurrent.Semaphore;

public class Questao3_BarbeiroDorminhoco {
    static final int CADEIRAS = 3;
    static int esperando = 0;

    static Semaphore clientes = new Semaphore(0);
    static Semaphore barbeiro = new Semaphore(0);
    static Semaphore mutex = new Semaphore(1);

    static class Cliente extends Thread {
        int id;
        Cliente(int id) { this.id = id; }

        public void run() {
            try {
                while (true) {
                    mutex.acquire();
                    if (esperando < CADEIRAS) {
                        esperando++;
                        System.out.println("Cliente " + id + " esperando. Total: " + esperando);
                        clientes.release();
                        mutex.release();
                        barbeiro.acquire();
                        System.out.println("Cliente " + id + " cortando cabelo.");
                        Thread.sleep(1000);
                    } else {
                        System.out.println("Cliente " + id + " foi embora, sem cadeiras.");
                        mutex.release();
                    }
                    Thread.sleep((int)(Math.random() * 3000 + 1000));
                }
            } catch (InterruptedException e) {}
        }
    }

    static class Barbeiro extends Thread {
        public void run() {
            try {
                while (true) {
                    clientes.acquire();
                    mutex.acquire();
                    esperando--;
                    System.out.println("Barbeiro cortando. Esperando agora: " + esperando);
                    mutex.release();
                    Thread.sleep(2000);
                    barbeiro.release();
                }
            } catch (InterruptedException e) {}
        }
    }

    public static void main(String[] args) {
        new Barbeiro().start();
        for (int i = 1; i <= 5; i++) {
            new Cliente(i).start();
        }
    }
}
