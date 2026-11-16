#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#define TAM_LINHA 1024

//struct para guardar os vetores e o estado geral dos recursos
typedef struct estado{
    //numero de clientes
    int n;
    int **maximum;
    int **allocation;
    int **need;
    int m;
    int *total;
    int *available;
} estado;

//algoritimo de seguranca, verifica se o estado atual e seguro, ou seja, se todos os clientes podem terminar
int is_safe(estado *e){
    int *work = malloc(e->m * sizeof(int));
    int *finish = calloc(e->n, sizeof(int));

    if (work == NULL || finish == NULL) {
        free(work);
        free(finish);
        return -1;
    }

    for (int j = 0; j < e->m; j++)
        work[j] = e->available[j];

    int terminados = 0;
    int achou = 1;

    //enquanto algum cliente conseguir terminar, devolve o que ele tem e repete
    while (achou) {
        achou = 0;
        for (int i = 0; i < e->n; i++) {
            if (finish[i])
                continue;

            int cabe = 1;
            for (int j = 0; j < e->m; j++) {
                if (e->need[i][j] > work[j]) {
                    cabe = 0;
                    break;
                }
            }

            if (cabe) {
                for (int j = 0; j < e->m; j++)
                    work[j] += e->allocation[i][j];
                finish[i] = 1;
                terminados++;
                achou = 1;
            }
        }
    }

    free(work);
    free(finish);
    return terminados == e->n;
}


int validacao (int argc, char const *argv[], estado *estado_sistema){
    //verifica se a quantidade de argumentos e valida
    if(argc < 2){
        fprintf(stderr, "uso: ./banker 10 5 7\n");
        return 1;
    }
    int m = argc - 1;
    estado_sistema->m = m;

    //vetor que vai representar a quantidade total de instancias disponiveis
    estado_sistema->total = malloc(m * sizeof(int));
    estado_sistema->available = malloc(m * sizeof(int));
    //checa se o malloc deu certo
    if (estado_sistema->total == NULL || estado_sistema->available == NULL) {
        fprintf(stderr, "Erro: falha de memoria\n");
        return 1;
    }

    //vai verificar cada argumento e adicionalo dentro do vetor responsavel por definir a quantidade de instancias
    for(int i = 0; i < m; i++){
        //aponta para o elemento que nao for um numero, para verificar se o argumento e um numero valido
        char *fim;
        //zera o errno antes da chamada, para detectar o ERANGE
        errno = 0;
        long valor = strtol(argv[i + 1], &fim, 10);

        if (fim == argv[i + 1] || *fim != '\0') {
            fprintf(stderr, "Erro: argumento '%s' nao e um numero valido\n", argv[i + 1]);
            return 1;
        }
        if (errno == ERANGE || valor < 0 || valor > INT_MAX) {
            fprintf(stderr, "Erro: argumento '%s' fora do intervalo permitido\n", argv[i + 1]);
            return 1;
        }
        //faz o casting de long para int e guarda nos dois vetores
        estado_sistema->total[i] = (int)valor;
        estado_sistema->available[i] = (int)valor;
    }

    return 0;
}

int validar_linha(const char *linha, int m, int *valores){
    const char *p = linha;

    for (int j = 0; j < m; j++) {
        //uso isdigit porque strtol aceita espacos em branco, mas nao quero aceitar
        if (!isdigit((unsigned char)*p))
            return -1;

        char *fim;
        errno = 0;
        long valor = strtol(p, &fim, 10);

        if (errno == ERANGE || valor > INT_MAX)
            return -1;

        if (valores != NULL)
            valores[j] = (int)valor;

        if (j < m - 1) {
            //ainda faltam numeros: tem que vir uma virgula logo depois
            if (*fim != ',')
                return -1;
            p = fim + 1;
        } else {
            //ultimo numero, a linha tem que acabar aqui
            //(sobrar virgula ou texto significa colunas a mais ou lixo)
            if (*fim != '\0')
                return -1;
        }
    }
    return 0;
}

//valida uma linha de comando
int validar_comando(const char *linha, int n, int m, char *tipo, int *cliente, int *valores){
    const char *p = linha;
    char t;

    //identifica o comando
    if (strncmp(p, "RQ", 2) == 0)
        t = 'Q';
    else if (strncmp(p, "RL", 2) == 0)
        t = 'L';
    else if (p[0] == '*')
        t = '*';
    else
        return -1;

    if (t == '*') {
        if (p[1] != '\0')
            return -1;
        if (tipo != NULL)
            *tipo = t;
        return 0;
    }

    //pula o RQ ou RL
    p += 2;

    //exige m + 1 numeros (cliente + m valores), cada um precedido de UM espaco
    for (int k = 0; k < m + 1; k++) {
        if (*p != ' ')
            return -1;
        p++;

        //isdigit novamente porque exige um digito valido
        if (!isdigit((unsigned char)*p))
            return -1;

        char *fim;
        errno = 0;
        long valor = strtol(p, &fim, 10);

        if (errno == ERANGE || valor > INT_MAX)
            return -1;

        if (k == 0) {
            if (valor >= n)
                return -1;
            if (cliente != NULL)
                *cliente = (int)valor;
        } else if (valores != NULL) {
            valores[k - 1] = (int)valor;
        }

        p = fim;
    }

    //a linha tem que acabar aqui, nao pode sobrar virgula ou texto
    if (*p != '\0')
        return -1;

    if (tipo != NULL)
        *tipo = t;
    return 0;
}

int **alocar_matriz(int n, int m){
    int **matriz = calloc(n, sizeof(int *));
    if (matriz == NULL)
        return NULL;

    for (int i = 0; i < n; i++) {
        matriz[i] = calloc(m, sizeof(int));
        if (matriz[i] == NULL) {
            for (int j = 0; j < i; j++)
                free(matriz[j]);
            free(matriz);
            return NULL;
        }
    }
    return matriz;
}

//funcao que le o arquivo de entrada customer.txt, valida e preenche a struct estado_sistema
int ler_customer(estado *estado_sistema){
    FILE *arquivo = fopen("customer.txt", "r");

    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir customer.txt\n");
        return -1;        // arquivo nao abriu, entao nao ha o que fechar
    }
    char linha[TAM_LINHA];
    int num_linha = 0;
    int clientes = 0;
    int linha_branca = 0;

    //primeira passada, conta o numero de linhas e valida o formato de cada linha
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        num_linha++;

        size_t tam = strlen(linha);

        //se nao terminou em '\n' e nao e o fim do arquivo, a linha estourou o buffer
        if (tam > 0 && linha[tam - 1] != '\n' && !feof(arquivo)) {
            fprintf(stderr, "Erro: customer.txt, linha %d: linha muito longa\n", num_linha);
            fclose(arquivo);
            return -1;
        }

        //remove '\n' e '\r' do final (arquivos do Windows)
        while (tam > 0 && (linha[tam - 1] == '\n' || linha[tam - 1] == '\r'))
            linha[--tam] = '\0';

        //linha em branco: so e permitida no final do arquivo
        if (tam == 0) {
            linha_branca = 1;
            continue;
        }

        //linha com conteudo depois de uma em branco = branca no meio
        if (linha_branca) {
            fprintf(stderr, "Erro: customer.txt, linha em branco antes da linha %d\n", num_linha);
            fclose(arquivo);
            return -1;
        }

        if (validar_linha(linha, estado_sistema->m, NULL) != 0) {
            fprintf(stderr, "Erro: customer.txt, linha %d: formato invalido (esperado %d numeros inteiros separados por virgula)\n", num_linha, estado_sistema->m);
            fclose(arquivo);
            return -1;
        }

        clientes++;
    }

    if (clientes == 0) {
        fprintf(stderr, "Erro: customer.txt esta vazio\n");
        fclose(arquivo);
        return -1;
    }

    estado_sistema->n = clientes;

    //allocacao das matrizes maximum, allocation e need
    estado_sistema->maximum = alocar_matriz(estado_sistema->n, estado_sistema->m);
    estado_sistema->allocation = alocar_matriz(estado_sistema->n, estado_sistema->m);
    estado_sistema->need = alocar_matriz(estado_sistema->n, estado_sistema->m);

    if (estado_sistema->maximum == NULL || estado_sistema->allocation == NULL || estado_sistema->need == NULL) {
        fprintf(stderr, "Erro: falha de memoria ao alocar as matrizes\n");
        fclose(arquivo);
        return -1;   // sem free: o main chama liberar_estado, que libera as que foram alocadas
    }
    //segunda passada, le os valores e preenche a matriz maximum, como a matriz ja foi validada na primeira passada
    //essa so faz preencher as matrizes, entao nao precisa validar novamente
    rewind(arquivo);  //volta para o inicio do arquivo
    //indice referente ao cliente atual
    int i = 0;
    //reaproveita o contador de linha da primeira passada, para dar mensagens de erro mais precisas
    num_linha = 0;

    while (i < estado_sistema->n && fgets(linha, sizeof(linha), arquivo) != NULL) {
        num_linha++;

        //size_t indica tamanho sem sinal, ou seja, qualquer inteiro positivo, muito utilizado para representar tamanhos
        //para essa situacao acaba sendo mais adequado do que um int
        size_t tam = strlen(linha);

        //remove '\n' e '\r' do final
        while (tam > 0 && (linha[tam - 1] == '\n' || linha[tam - 1] == '\r'))
            linha[--tam] = '\0';

        //como o arquivo ja foi validado, uma linha em branco nessa caso representa o final
        if (tam == 0)
            continue;

        //guarda os numeros em maximum[i]
        if (validar_linha(linha, estado_sistema->m, estado_sistema->maximum[i]) != 0) {
            fprintf(stderr, "Erro: customer.txt, linha %d: formato invalido\n", num_linha);
            fclose(arquivo);
            return -1;
        }

        for (int j = 0; j < estado_sistema->m; j++) {
            //o maximo de um cliente nao pode passar do total do recurso
            if (estado_sistema->maximum[i][j] > estado_sistema->total[j]) {
                fprintf(stderr, "Erro: customer.txt, linha %d: cliente %d, recurso %d: maximo %d maior que o total %d\n",num_linha, i, j,estado_sistema->maximum[i][j], estado_sistema->total[j]);
                fclose(arquivo);
                return -1;
            }
            //por contqa do calloc allocation ja esta zerada entao need = maximum
            estado_sistema->need[i][j] = estado_sistema->maximum[i][j];
        }

        i++;
    }

    //confere que carregou exatamente n clientes
    if (i != estado_sistema->n) {
        fprintf(stderr, "Erro: customer.txt mudou durante a leitura\n");
        fclose(arquivo);
        return -1;
    }

    fclose(arquivo);
    return 0;
}


void imprimir_estado(estado *estado_sistema, FILE *saida){
    fprintf(saida, "MAXIMUM   |   ALLOCATION  |   NEED\n");

    for (int i = 0; i < estado_sistema->n; i++) {
        int escritos;

        // 1a coluna (maximum): imprime os valores e completa ate 10 caracteres
        escritos = 0;
        for (int j = 0; j < estado_sistema->m; j++) {
            if (j > 0)
                escritos += fprintf(saida, " ");
            escritos += fprintf(saida, "%d", estado_sistema->maximum[i][j]);
        }
        // pelo menos um espaco antes do '|', mesmo se a coluna estourar a largura
        do {
            fprintf(saida, " ");
            escritos++;
        } while (escritos < 10);
        fprintf(saida, "|   ");

        // 2a coluna (allocation): contador zerado, completa ate 12 caracteres
        escritos = 0;
        for (int j = 0; j < estado_sistema->m; j++) {
            if (j > 0)
                escritos += fprintf(saida, " ");
            escritos += fprintf(saida, "%d", estado_sistema->allocation[i][j]);
        }
        do {
            fprintf(saida, " ");
            escritos++;
        } while (escritos < 12);
        fprintf(saida, "|   ");

        // 3a coluna (need): sem preenchimento, mas com um espaco no fim da linha,
        // como no arquivo do professor
        for (int j = 0; j < estado_sistema->m; j++) {
            if (j > 0)
                fprintf(saida, " ");
            fprintf(saida, "%d", estado_sistema->need[i][j]);
        }
        fprintf(saida, " \n");
    }

    // o AVAILABLE tambem termina com um espaco no arquivo do professor
    fprintf(saida, "AVAILABLE");
    for (int j = 0; j < estado_sistema->m; j++)
        fprintf(saida, " %d", estado_sistema->available[j]);
    fprintf(saida, " \n");
}

//funcao responsavel por mostrar mensagens de saida que tem um vetor no meio
void imprimir_vetor(FILE *saida, const int *v, int m){
    for (int j = 0; j < m; j++) {
        if (j > 0)
            fprintf(saida, " ");
        fprintf(saida, "%d", v[j]);
    }
}

//libera a struct
void liberar_matriz(int **matriz, int n){
    if (matriz == NULL){
        return;
    }
    for (int i = 0; i < n; i++){
        free(matriz[i]); 
    }
    free(matriz);
}

void liberar_estado(estado *estado_sistema){
    free(estado_sistema->total);
    free(estado_sistema->available);
    estado_sistema->total = NULL;
    estado_sistema->available = NULL;

    liberar_matriz(estado_sistema->maximum, estado_sistema->n);
    liberar_matriz(estado_sistema->allocation, estado_sistema->n);
    liberar_matriz(estado_sistema->need, estado_sistema->n);
    estado_sistema->maximum = NULL;
    estado_sistema->allocation = NULL;
    estado_sistema->need = NULL;
}

//trata o RL
//libera oque o cliente tem alocado, e devolve para o vetor available
void release(estado *e, int c, const int *req, FILE *saida){
    //o cliente so pode liberar o que tem alocado
    for (int j = 0; j < e->m; j++) {
        if (req[j] > e->allocation[c][j]) {
            fprintf(saida, "The customer %d release ", c);
            imprimir_vetor(saida, req, e->m);
            fprintf(saida, " was denied because exceed its allocation\n");
            return;
        }
    }

    for (int j = 0; j < e->m; j++) {
        e->available[j] += req[j];
        e->allocation[c][j] -= req[j];
        e->need[c][j] += req[j];
    }

    fprintf(saida, "Release from customer %d the resources ", c);
    imprimir_vetor(saida, req, e->m);
    fprintf(saida, "\n");
}

//trata RQ
//e a responsavel por aceitar ou negar o pedido de um cliente
int request(estado *e, int c, const int *req, FILE *saida){
    //pedido nao pode passar da necessidade do cliente
    for (int j = 0; j < e->m; j++) {
        if (req[j] > e->need[c][j]) {
            fprintf(saida, "The customer %d request ", c);
            imprimir_vetor(saida, req, e->m);
            fprintf(saida, " was denied because exceed its maximum allocation\n");
            return 0;
        }
    }

    //pedido nao pode passar do que esta disponivel
    for (int j = 0; j < e->m; j++) {
        if (req[j] > e->available[j]) {
            fprintf(saida, "The resources ");
            imprimir_vetor(saida, e->available, e->m);
            fprintf(saida, " are not enough to customer %d request ", c);
            imprimir_vetor(saida, req, e->m);
            fprintf(saida, " \n");
            return 0;
        }
    }

    //simula a alocacao
    for (int j = 0; j < e->m; j++) {
        e->available[j] -= req[j];
        e->allocation[c][j] += req[j];
        e->need[c][j] -= req[j];
    }

    //checa se o estado resultante e seguro
    int seguro = is_safe(e);

    //caso nao passe no check desfaz a simulacao
    if (seguro != 1) {
        for (int j = 0; j < e->m; j++) {
            e->available[j] += req[j];
            e->allocation[c][j] -= req[j];
            e->need[c][j] += req[j];
        }
        if (seguro < 0)
            return -1;

        fprintf(saida, "The customer %d request ", c);
        imprimir_vetor(saida, req, e->m);
        fprintf(saida, " was denied because result in an unsafe state\n");
        return 0;
    }

    //caso contrario, a operacao e liberada
    fprintf(saida, "Allocate to customer %d the resources ", c);
    imprimir_vetor(saida, req, e->m);
    fprintf(saida, "\n");
    return 0;
}

//funcao responsavel por ler o arquivo commands.txt, validar e executar os comandos
int processar_commands(estado *estado_sistema, FILE *saida){
    FILE *arquivo = fopen("commands.txt", "r");

    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir commands.txt\n");
        return -1;        // arquivo nao abriu, entao nao ha o que fechar
    }

    char linha[TAM_LINHA];
    int num_linha = 0;
    //contador de comandos validos
    int comandos = 0;
    //flag para linhas brancas
    int linha_branca = 0;  
    //primeira passada, valida o arquivo antes da execucao na segunda passada
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        num_linha++;

        size_t tam = strlen(linha);

        //se nao terminou em '\n' e nao e o fim do arquivo, a linha estourou o buffer
        if (tam > 0 && linha[tam - 1] != '\n' && !feof(arquivo)) {
            fprintf(stderr, "Erro: commands.txt, linha %d: linha muito longa\n", num_linha);
            fclose(arquivo);
            return -1;
        }

        //remove '\n' e '\r' do final
        while (tam > 0 && (linha[tam - 1] == '\n' || linha[tam - 1] == '\r'))
            linha[--tam] = '\0';

        //lina em branco so e permitida no final do arquivo
        if (tam == 0) {
            linha_branca = 1;
            continue;
        }

        if (linha_branca) {
            fprintf(stderr, "Erro: commands.txt, linha em branco antes da linha %d\n", num_linha);
            fclose(arquivo);
            return -1;
        }

        if (validar_comando(linha, estado_sistema->n, estado_sistema->m, NULL, NULL, NULL) != 0) {
            fprintf(stderr, "Erro: commands.txt, linha %d: comando invalido (esperado RQ/RL cliente + %d valores, ou *; cliente entre 0 e %d)\n", num_linha, estado_sistema->m, estado_sistema->n - 1);
            fclose(arquivo);
            return -1;
        }

        comandos++;
    }

    if (comandos == 0) {
        fprintf(stderr, "Erro: commands.txt nao tem nenhum comando\n");
        fclose(arquivo);
        return -1;
    }
    
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        num_linha++;
        comandos++;
    }
    //segunda passada, executa os comandos
    rewind(arquivo);

    int *valores = calloc(estado_sistema->m, sizeof(int));
    if (valores == NULL) {
        fprintf(stderr, "Erro: falha de memoria\n");
        fclose(arquivo);
        return -1;
    }

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        size_t tam = strlen(linha);

        //remove '\n' e '\r' do final
        while (tam > 0 && (linha[tam - 1] == '\n' || linha[tam - 1] == '\r'))
            linha[--tam] = '\0';

        //o arquivo ja foi validado, linhas em branco so podem indicar o final
        if (tam == 0)
            continue;

        char tipo;
        int cliente = 0;

        //valida de novo e extrai o tipo, o cliente e os valores
        if (validar_comando(linha, estado_sistema->n, estado_sistema->m,&tipo, &cliente, valores) != 0) {
            fprintf(stderr, "Erro: commands.txt mudou durante a leitura\n");
            free(valores);
            fclose(arquivo);
            return -1;
        }

        if (tipo == 'Q') {
            if (request(estado_sistema, cliente, valores, saida) != 0) {
                fprintf(stderr, "Erro: falha de memoria\n");
                free(valores);
                fclose(arquivo);
                return -1;
            }
        } else if (tipo == 'L') {
            release(estado_sistema, cliente, valores, saida);
        } else {
            imprimir_estado(estado_sistema, saida);
        }
    }

    free(valores);
    fclose(arquivo);
    return 0;
}

int main(int argc, char const *argv[]){
    //struct estado_sistema e zerada (fazendo todos atributos comecarem com NULL)
    estado estado_sistema = {0};

    int retorno = validacao(argc, argv, &estado_sistema);
    //verifica se a validacao falhou
    if (retorno != 0) {
        liberar_estado(&estado_sistema);
        return 1;
    }

    retorno = ler_customer(&estado_sistema);
    if (retorno != 0) {
        liberar_estado(&estado_sistema);
        return 1;
    }

    //abre o arquivo de saida em modo "w" (cria ou sobrescreve um result.txt antigo)
    FILE *saida = fopen("result.txt", "w");
    if (saida == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar result.txt\n");
        liberar_estado(&estado_sistema);
        return 1;
    }

    //le o commands.txt, valida tudo e executa os comandos, gravando em result.txt
    retorno = processar_commands(&estado_sistema, saida);
    fclose(saida);

    if (retorno != 0) {
        //a execucao falhou: apaga o result.txt para nao deixar um arquivo vazio ou incompleto
        remove("result.txt");
        liberar_estado(&estado_sistema);
        return 1;
    }

    liberar_estado(&estado_sistema);
    return 0;
}