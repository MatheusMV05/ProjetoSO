# Estrutura do Código

Pessoa.java: Classe base que contém o ID único para todas as pessoas (barbeiros e clientes).
Cliente.java: Representa um cliente que deseja cortar o cabelo. Ele funciona como uma thread que tenta entrar na barbearia periodicamente (com intervalos de 3-5 segundos entre tentativas).
Barbeiro.java: Representa um barbeiro que atende os clientes. Cada barbeiro é uma thread que busca o próximo cliente, realiza o corte (simulado com um tempo entre 1-3 segundos) e finaliza o atendimento.
Barbearia.java: Implementa a lógica de sincronização usando monitores Java. Ela gerencia as cadeiras de espera, barbeiros disponíveis e garante a ordem de chegada dos clientes.
Main.java: Classe principal que inicializa a simulação com os parâmetros fornecidos pela linha de comando (número de barbeiros, cadeiras e clientes).

Funcionamento

Quando um cliente tenta entrar na barbearia (cortaCabelo), ele verifica se há espaço:

Se a barbearia estiver lotada (todos os barbeiros ocupados e cadeiras de espera cheias), ele sai.
Se houver um barbeiro livre, ele é atendido imediatamente.
Se não houver barbeiros livres, mas houver cadeiras de espera disponíveis, ele entra na fila.

Os barbeiros ficam em loop chamando proximoCliente():

Se não houver clientes na fila, o barbeiro "dorme" (wait) até ser notificado.
Quando há clientes, ele atende o próximo da fila.

Após concluir o corte, o barbeiro chama corteTerminado() para liberar o cliente.

Características Importantes

A implementação controla corretamente os casos onde todos os barbeiros estão dormindo ou ocupados.
A ordem de chegada dos clientes é respeitada através da fila FIFO.
Os clientes são acordados especificamente quando seu corte termina.
A simulação imprime mensagens informativas conforme solicitado.

Compilação e Execução

Para compilar os arquivos:
javac *.java

Para executar (exemplo com 2 barbeiros, 3 cadeiras e 10 clientes):
java Main 2 3 10
