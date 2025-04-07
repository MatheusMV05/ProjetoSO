import java.util.Random;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class Cliente extends Pessoa implements Runnable {
    private final Barbearia barbearia;
    private final Random random;
    private boolean ativo;

    public Cliente(int id, Barbearia barbearia) {
        super(id);
        this.barbearia = barbearia;
        this.random = new Random();
        this.ativo = true;
    }

    @Override
    public void run() {
        ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();

        Runnable task = () -> {
            if (!ativo) {
                scheduler.shutdown();
                return;
            }

            boolean atendido = barbearia.cortaCabelo(this);

            if (!atendido) {
                System.out.println(
                        "Cliente " + id + " tentou entrar na barbearia, mas está lotada... indo dar uma voltinha");
            }
        };

        scheduler.scheduleWithFixedDelay(task, 0, random.nextInt(2000) + 3000, TimeUnit.MILLISECONDS);
    }

    public void parar() {
        ativo = false;
    }
}