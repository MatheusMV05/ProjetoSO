import java.util.LinkedList;
import java.util.Queue;

public class Barbearia {
    private final int cadeirasEspera;
    private final int numBarbeiros;
    private int barbeirosLivres;
    private final Queue<Cliente> filaEspera;
    private final Cliente[] clientesEmAtendimento;

    public Barbearia(int numBarbeiros, int cadeirasEspera) {
        this.numBarbeiros = numBarbeiros;
        this.cadeirasEspera = cadeirasEspera;
        this.barbeirosLivres = numBarbeiros;
        this.filaEspera = new LinkedList<>();
        this.clientesEmAtendimento = new Cliente[numBarbeiros];
    }

    public synchronized boolean cortaCabelo(Cliente cliente) {
        if (barbeirosLivres == 0 && filaEspera.size() >= cadeirasEspera) {
            return false;
        }

        System.out.println("Cliente " + cliente.getID() + " entrou na barbearia");

        if (barbeirosLivres > 0) {
            barbeirosLivres--;
            System.out.println("Cliente " + cliente.getID() + " será atendido imediatamente");
            notifyAll();

            try {
                System.out.println("Cliente " + cliente.getID() + " esperando corte...");
                wait();
            } catch (InterruptedException e) {
                System.err.println("Thread interrupted: " + e.getMessage());
            }

            System.out.println("Cliente " + cliente.getID() + " terminou o corte... saindo da barbearia!");
            return true;
        } else {
            filaEspera.add(cliente);
            System.out.println("Cliente " + cliente.getID() + " esperando na fila. Posição: " + filaEspera.size());

            try {
                wait();
            } catch (InterruptedException e) {
                System.err.println("Erro ao aguardar na fila: " + e.getMessage());
            }

            System.out.println("Cliente " + cliente.getID() + " terminou o corte... saindo da barbearia!");
            return true;
        }
    }

    public synchronized Cliente proximoCliente() {
        if (filaEspera.isEmpty()) {
            barbeirosLivres++;

            if (barbeirosLivres == numBarbeiros) {
                System.out.println("Barbeiro " + Thread.currentThread().getName()
                        + " indo dormir um pouco... não há clientes na barbearia...");
            } else {
                System.out.println("Barbeiro " + Thread.currentThread().getName()
                        + " indo dormir um pouco... aguardando clientes");
            }

            try {
                wait();
            } catch (InterruptedException e) {
                System.err.println("Erro ao aguardar cliente: " + e.getMessage());
            }

            barbeirosLivres--;
            System.out.println("Barbeiro " + Thread.currentThread().getName() + " acordou! Começando os trabalhos!");
        }

        if (!filaEspera.isEmpty()) {
            Cliente cliente = filaEspera.poll();
            int barbeiroPosicao = Integer.parseInt(Thread.currentThread().getName());
            clientesEmAtendimento[barbeiroPosicao] = cliente;

            return cliente;
        }

        return null;
    }

    public synchronized void corteTerminado(Cliente cliente) {
        int barbeiroPosicao = -1;
        for (int i = 0; i < clientesEmAtendimento.length; i++) {
            if (clientesEmAtendimento[i] != null && clientesEmAtendimento[i].getID() == cliente.getID()) {
                barbeiroPosicao = i;
                break;
            }
        }

        if (barbeiroPosicao != -1) {
            clientesEmAtendimento[barbeiroPosicao] = null;
            notifyAll();
        }
    }
}