#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

//struct para guardar os vetores e o estado geral dos recursos
typedef struct estado{
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

//libera a struct
void liberar_estado(estado *estado_sistema){
    free(estado_sistema->total);
    free(estado_sistema->available);
    estado_sistema->total = NULL;
    estado_sistema->available = NULL;
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