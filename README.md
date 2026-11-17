# Algoritmo do Banqueiro (Banker's Algorithm)

Implementação em C do **Algoritmo do Banqueiro** para gerenciamento e prevenção de *deadlocks* na alocação dinâmica de recursos entre múltiplos clientes (processos).

---

## Visão Geral

O programa recebe pela linha de comando a quantidade total de instâncias disponíveis para cada tipo de recurso ($m \ge 1$), lê a demanda máxima de cada cliente a partir de `customer.txt` e processa sequencialmente as requisições (`RQ`), liberações (`RL`) e consultas de estado (`*`) descritas em `commands.txt`.

Antes de conceder qualquer requisição de recursos, o simulador executa o algoritmo de segurança (`is_safe`), garantindo que o sistema permaneça sempre em um **estado seguro** (isto é, que exista pelo menos uma sequência de execução na qual todos os clientes consigam concluir suas tarefas sem impasse).

---



## Estrutura do Projeto

```text
Algoritmo-de-Banker/
├── banker.c        # Código-fonte principal (validação, leitura, algoritmo de segurança e I/O)
├── Makefile        # Automação de compilação e limpeza
├── customer.txt    # Matriz Maximum de cada cliente (entrada)
├── commands.txt    # Sequência de comandos RQ, RL e * (entrada)
├── result.txt      # Saída gerada após a execução 

```

---

## Compilação

No terminal Ubuntu (WSL), navegue até o diretório raiz do projeto e execute:

```bash
make
```

O `Makefile` compila o código com as flags `-Wall -Wextra -std=c11 -g` e gera o executável `./banker`.

Para remover o binário e o arquivo `result.txt`:

```bash
make clean
```

---

## Formato dos Arquivos de Entrada

### 1. `customer.txt`
Define a matriz **Maximum** de cada cliente. Deve conter $n$ linhas (uma por cliente, indexados de `0` a `n - 1`), cada uma com $m$ números inteiros não negativos separados por vírgula (compatível com quebras de linha Unix `LF` e Windows `CRLF`):

```text
7,5,3
3,2,2
9,0,2
2,2,2
4,3,3
```

### 2. `commands.txt`
Define as operações a serem processadas em ordem, uma por linha:

* **`RQ <cliente> <r_0> <r_1> ... <r_{m-1}>`**: Solicita recursos para o `<cliente>`.
* **`RL <cliente> <r_0> <r_1> ... <r_{m-1}>`**: Libera recursos alocados pelo `<cliente>`.
* **`*`**: Imprime o estado atual das matrizes `MAXIMUM`, `ALLOCATION`, `NEED` e do vetor `AVAILABLE`.

Exemplo:

```text
RQ 0 0 2 1
RQ 2 3 0 2
RL 0 0 1 1
*
```

---

## Como Executar

Informe como argumentos da linha de comando a quantidade inicial disponível de cada um dos $m$ tipos de recursos:

```bash
./banker <qtd_recurso_0> <qtd_recurso_1> ... <qtd_recurso_{m-1}>
```

### Exemplo

```bash
./banker 10 5 7
```

Para visualizar o resultado gerado:

```bash
cat result.txt
```

---

## Saída e Tratamento de Erros

* **Execução válida (código de saída `0`):** Toda a saída da simulação é gravada em **`result.txt`** no diretório atual.
  * Requisições concedidas registram `Allocate to customer <c> the resources ...`.
  * Requisições negadas registram o motivo específico:
    * Excesso sobre a necessidade máxima (`exceed its maximum allocation`);
    * Indisponibilidade momentânea (`are not enough to customer <c> request ...`);
    * Risco de impasse (`was denied because result in an unsafe state`).
  * Liberações válidas registram `Release from customer <c> the resources ...`, enquanto tentativas acima do alocado são negadas (`exceed its allocation`).
* **Erro de entrada ou validação (código de saída `1`):** Caso os argumentos ou arquivos de entrada estejam ausentes, vazios ou malformados, uma mensagem descritiva é exibida na saída padrão de erro (**`stderr`**) e nenhum arquivo `result.txt` incompleto é mantido no diretório.
## Ambiente de Desenvolvimento
Este projeto foi desenvolvido e validado utilizando **Ubuntu via WSL (Windows Subsystem for Linux)** no **Windows**.




---
## Autor

* **Juan Riquelme Xavier de Souza**
