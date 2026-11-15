#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

void validacao_args(int argc, char const *argv[]){
    //verifica se a quantidade de argumentos e valida
    if(argc < 2){
        printf("uso: ./banker 10 5 7\n");
        return;
    }
    int n = argc - 1;
    //vetor que vai representar a quantidade total de instancias disponiveis
    int *total = malloc(n * sizeof(int));
    //checa se o malloc deu certo
    if (total == NULL) {
        fprintf(stderr, "Erro: falha de memoria\n");
        return;
    }

    //vai verificar cada argumento e adicionalo dentro do vetor responsavel por definir a quantidade de instancias
    for(int i = 0; i < n; i++){
        //aponta para o elemento qua nao for um numero, para verificar se o argumento e um numero valido
        char *fim;
        //aponta caso ocorra algum erro como o erange que e o mais relevante nesse caso
        errno = 0;
        long valor = strtol(argv[i +1], &fim, 10);

        if (fim == argv[i + 1] || *fim != '\0') {
            fprintf(stderr, "Erro: argumento '%s' nao e um numero valido\n", argv[i + 1]);
            free(total);
            return;
        }
        if (errno == ERANGE || valor < 0 || valor > INT_MAX) {
            fprintf(stderr, "Erro: argumento '%s' fora do intervalo permitido\n", argv[i + 1]);
            free(total);
            return;
        }
        //faz o casting de valor, de long para int
        //e adiciona aquele valor dentro do vetor total
        total[i] = (int)valor;
    }
    printf("%d recursos:", n);
    for (int i = 0; i < n; i++)
        printf(" %d", total[i]);
    printf("\n");

    free(total);
}

int main(int argc, char const *argv[]){
    validacao_args(argc,argv);
    return 0;
}