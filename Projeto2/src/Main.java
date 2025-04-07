public class Main {
    public static void main(String[] args) {
        if (args.length < 3) {
            System.out.println("Uso: java Main <número de barbeiros> <número de cadeiras> <número de clientes>");
            return;
        }

        try {
            int numBarbeiros = Integer.parseInt(args[0]);
            int numCadeiras = Integer.parseInt(args[1]);
            int numClientes = Integer.parseInt(args[2]);

            if (numBarbeiros <= 0 || numCadeiras < 0 || numClientes <= 0) {
                System.out.println("Os valores devem ser positivos!");
                return;
            }

            System.out.println("Iniciando barbearia com " + numBarbeiros + " barbeiros, "
                    + numCadeiras + " cadeiras de espera e " + numClientes + " clientes.");

            Barbearia barbearia = new Barbearia(numBarbeiros, numCadeiras);

            Thread[] barbeirosThreads = new Thread[numBarbeiros];
            Barbeiro[] barbeiros = new Barbeiro[numBarbeiros];

            for (int i = 0; i < numBarbeiros; i++) {
                barbeiros[i] = new Barbeiro(i, barbearia);
                barbeirosThreads[i] = new Thread(barbeiros[i], String.valueOf(i));
                barbeirosThreads[i].start();
            }

            Thread[] clientesThreads = new Thread[numClientes];
            Cliente[] clientes = new Cliente[numClientes];

            java.util.concurrent.ScheduledExecutorService executor = java.util.concurrent.Executors
                    .newScheduledThreadPool(1);

            for (int i = 0; i < numClientes; i++) {
                final int clientId = i;
                executor.schedule(() -> {
                    clientes[clientId] = new Cliente(clientId, barbearia);
                    clientesThreads[clientId] = new Thread(clientes[clientId]);
                    clientesThreads[clientId].start();
                }, i * 100, java.util.concurrent.TimeUnit.MILLISECONDS);
            }

            executor.shutdown();
            executor.awaitTermination(numClientes * 100 + 1000, java.util.concurrent.TimeUnit.MILLISECONDS);

            for (int i = 0; i < numClientes; i++) {
                clientesThreads[i].join();
            }

            System.out.println("Simulação concluída. Todos os clientes foram atendidos ou desistiram.");

            for (int i = 0; i < numBarbeiros; i++) {
                barbeiros[i].parar();
            }

            for (int i = 0; i < numBarbeiros; i++) {
                barbeirosThreads[i].interrupt();
            }

        } catch (NumberFormatException e) {
            System.out.println("Erro: os argumentos devem ser números!");
        } catch (InterruptedException e) {
            System.out.println("Simulação interrompida.");
        }
    }
}