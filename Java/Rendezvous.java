public class Rendezvous {
    static final Object lock = new Object();
    static boolean arrivedA = false, arrivedB = false;

    static class ThreadA extends Thread {
        public void run() {
            synchronized(lock) {
                System.out.println("Thread A chegou no rendez-vous.");
                arrivedA = true;
                lock.notifyAll();
                while (!arrivedB) {
                    try { 
                        lock.wait(); 
                    } catch (InterruptedException e) { }
                }
            }
            System.out.println("Thread A prossegue.");
        }
    }

    static class ThreadB extends Thread {
        public void run() {
            synchronized(lock) {
                System.out.println("Thread B chegou no rendez-vous.");
                arrivedB = true;
                lock.notifyAll();
                while (!arrivedA) {
                    try { 
                        lock.wait(); 
                    } catch (InterruptedException e) { }
                }
            }
            System.out.println("Thread B prossegue.");
        }
    }

    public static void main(String[] args) {
        new ThreadA().start();
        new ThreadB().start();
    }
}
