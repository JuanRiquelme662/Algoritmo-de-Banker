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
        // o que tiver sido alocado (mesmo se so um dos dois deu certo)
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
        //uso isdigit porque strtol aceita espacos em branco, mas nao queremos aceitar
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
            // ainda faltam numeros: tem que vir uma virgula logo depois
            if (*fim != ',')
                return -1;
            p = fim + 1;
        } else {
            // ultimo numero: a linha tem que acabar aqui
            // (sobrar virgula ou texto significa colunas a mais ou lixo)
            if (*fim != '\0')
                return -1;
        }
    }
    return 0;
}

int ler_customer(estado *estado_sistema){
    int resultado = -1;   
    FILE *arquivo = fopen("customer.txt", "r");

    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir customer.txt\n");
        return -1;        
    }
    resultado = 0;        
    fclose(arquivo);      
    return resultado;
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

int main(int argc, char const *argv[]){
    //struct estado_sistema e zerada (fazendo todos atributos comecarem com NULL)
    estado estado_sistema = {0};

    int retorno = validacao(argc, argv, &estado_sistema);
    //verifica se a validacao falhou
    if (retorno != 0) {
        liberar_estado(&estado_sistema);
        return 1;
    }
    //impressao para debug
    printf("%d recursos:", estado_sistema.m);
    for (int i = 0; i < estado_sistema.m; i++)
        printf(" %d", estado_sistema.total[i]);
    printf("\n");
    liberar_estado(&estado_sistema);
    return 0;
}