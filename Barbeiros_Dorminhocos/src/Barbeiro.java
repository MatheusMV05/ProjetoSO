import java.util.Random;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class Barbeiro extends Pessoa implements Runnable {
    private final Barbearia barbearia;
    private final Random random;
    private boolean ativo;

    public Barbeiro(int id, Barbearia barbearia) {
        super(id);
        this.barbearia = barbearia;
        this.random = new Random();
        this.ativo = true;
    }

    @Override
    public void run() {
        System.out.println("Barbeiro " + id + " começou a trabalhar!");

        while (ativo) {
            Cliente cliente = barbearia.proximoCliente();

            if (cliente != null) {
                System.out.println("Cliente " + cliente.getID() + " cortando cabelo com Barbeiro " + id);

                try {
                    int delay = random.nextInt(2000) + 1000;
                    ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();
                    CountDownLatch latch = new CountDownLatch(1);
                    scheduler.schedule(() -> {
                        latch.countDown();
                        scheduler.shutdown();
                    }, delay, TimeUnit.MILLISECONDS);
                    latch.await();
                } catch (InterruptedException e) {
                    System.err.println("Thread interrupted: " + e.getMessage());
                }

                barbearia.corteTerminado(cliente);
            }
        }
    }

    public void parar() {
        ativo = false;
    }
}