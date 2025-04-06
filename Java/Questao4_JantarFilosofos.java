import java.util.concurrent.Semaphore;

public class Questao4_JantarFilosofos {
    static final int N = 5;
    static Semaphore[] garfos = new Semaphore[N];

    static class Filosofo extends Thread {
        int id;
        Filosofo(int id) { this.id = id; }

        public void run() {
            try {
                while (true) {
                    System.out.println("Filósofo " + id + " pensando...");
                    Thread.sleep(1000);

                    garfos[id].acquire();
                    garfos[(id+1)%N].acquire();

                    System.out.println("Filósofo " + id + " comendo...");
                    Thread.sleep(2000);

                    garfos[id].release();
                    garfos[(id+1)%N].release();

                    System.out.println("Filósofo " + id + " terminou de comer.");
                }
            } catch (InterruptedException e) {}
        }
    }

    public static void main(String[] args) {
        for (int i = 0; i < N; i++)
            garfos[i] = new Semaphore(1);

        for (int i = 0; i < N; i++)
            new Filosofo(i).start();
    }
}
