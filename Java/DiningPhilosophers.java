import java.util.concurrent.Semaphore;

class Philosopher extends Thread {
    private int id;
    private Semaphore leftChopstick, rightChopstick, saleiro;

    public Philosopher(int id, Semaphore left, Semaphore right, Semaphore saleiro) {
        this.id = id;
        this.leftChopstick = left;
        this.rightChopstick = right;
        this.saleiro = saleiro;
    }

    public void run() {
        try {
            while (true) {
                System.out.println("Filósofo " + id + " meditando.");
                Thread.sleep(1000);
                saleiro.acquire();
                leftChopstick.acquire();
                rightChopstick.acquire();
                System.out.println("Filósofo " + id + " comendo.");
                Thread.sleep(2000);
                leftChopstick.release();
                rightChopstick.release();
                saleiro.release();
            }
        } catch (InterruptedException e) { }
    }
}

public class DiningPhilosophers {
    public static void main(String[] args) {
        int numPhilos = 5;
        Semaphore[] chopsticks = new Semaphore[numPhilos];
        for (int i = 0; i < numPhilos; i++) {
            chopsticks[i] = new Semaphore(1);
        }
        // Permite que no máximo 4 filósofos tentem comer simultaneamente
        Semaphore saleiro = new Semaphore(numPhilos - 1);

        Philosopher[] philosophers = new Philosopher[numPhilos];
        for (int i = 0; i < numPhilos; i++) {
            Semaphore left = chopsticks[i];
            Semaphore right = chopsticks[(i + 1) % numPhilos];
            philosophers[i] = new Philosopher(i, left, right, saleiro);
            philosophers[i].start();
        }
    }
}
