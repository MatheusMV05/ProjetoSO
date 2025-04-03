import java.util.concurrent.Semaphore;

public class ThreeRobots {
    static Semaphore semBart = new Semaphore(1);
    static Semaphore semLisa1 = new Semaphore(0);
    static Semaphore semMaggie = new Semaphore(0);
    static Semaphore semLisa2 = new Semaphore(0);

    static class Bart extends Thread {
        public void run() {
            while (true) {
                try {
                    semBart.acquire();
                    System.out.println("Bart executando...");
                    Thread.sleep(1000);
                    semLisa1.release();
                } catch (InterruptedException e) { }
            }
        }
    }

    static class Lisa extends Thread {
        public void run() {
            while (true) {
                try {
                    semLisa1.acquire();
                    System.out.println("Lisa executando - turno 1...");
                    Thread.sleep(1000);
                    semMaggie.release();
                    semLisa2.acquire();
                    System.out.println("Lisa executando - turno 2...");
                    Thread.sleep(1000);
                    semBart.release();
                } catch (InterruptedException e) { }
            }
        }
    }

    static class Maggie extends Thread {
        public void run() {
            while (true) {
                try {
                    semMaggie.acquire();
                    System.out.println("Maggie executando...");
                    Thread.sleep(1000);
                    semLisa2.release();
                } catch (InterruptedException e) { }
            }
        }
    }

    public static void main(String[] args) {
        new Bart().start();
        new Lisa().start();
        new Maggie().start();
    }
}
