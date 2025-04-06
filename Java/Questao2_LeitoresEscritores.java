import java.util.concurrent.Semaphore;

public class Questao2_LeitoresEscritores {
    static int data = 0, rc = 0;
    static Semaphore mutex = new Semaphore(1);
    static Semaphore writeblock = new Semaphore(1);

    static class Leitor extends Thread {
        int id;
        Leitor(int id) { this.id = id; }

        public void run() {
            try {
                while (true) {
                    mutex.acquire();
                    rc++;
                    if (rc == 1)
                        writeblock.acquire();
                    mutex.release();

                    System.out.println("Leitor " + id + " leu: " + data);
                    Thread.sleep(1000);

                    mutex.acquire();
                    rc--;
                    if (rc == 0)
                        writeblock.release();
                    mutex.release();

                    Thread.sleep(2000);
                }
            } catch (InterruptedException e) {}
        }
    }

    static class Escritor extends Thread {
        int id;
        Escritor(int id) { this.id = id; }

        public void run() {
            try {
                while (true) {
                    writeblock.acquire();
                    data++;
                    System.out.println("Escritor " + id + " escreveu: " + data);
                    Thread.sleep(2000);
                    writeblock.release();
                    Thread.sleep(3000);
                }
            } catch (InterruptedException e) {}
        }
    }

    public static void main(String[] args) {
        new Leitor(1).start();
        new Leitor(2).start();
        new Escritor(1).start();
    }
}
