#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct { //parametros que serão utilizados nas threads de leitura
    int idThread;
    char* arquivo;
    int* vetor;
} parametrosLeitura;

void* leitura(void* arg){
    
    parametrosLeitura* parametros = (parametrosLeitura*)arg;

    int num = parametros->idThread;
    int indiceVetor = ((num-1) * 1000); // cada thread vai preencher mil posições do vetor

    FILE* arquivo = fopen(parametros->arquivo, "r"); // abre o arquivo de cada parametro no modo de leitura

    if (arquivo == NULL){
        printf("Não abriu o arquivo %s\n", parametros->arquivo);
        return NULL;
    }
    
    while (fscanf(arquivo, "%d", &parametros->vetor[indiceVetor]) != EOF) { // Salva no vetor os números do arquivo
        indiceVetor++;
    }
    
    fclose(arquivo);
    pthread_exit(NULL); // Finaliza a thread
}

int main(int argc, char const *argv[]) // argv[1] = num de nucleos, os demais são os arquivos de numeros, argv[argc] = nome do arquivo de saida
{
    

    pthread_t leitor[2]; // considera que 2 threads iram ler e salvar

    int quantidade = (argc - 2)*1000; // rever em função da parte -o saida.txt
    int* vetorPrincipal = (int*)malloc(quantidade * sizeof(int)); // vetor alocado dinamicamente com o tamanho para cada arquivo q for abrir


    for (int i = 1; i < argc-1; i++){ // argc conta a execução(nome) do arquivo como parametro, porem argv começa no 0 cuidado

        parametrosLeitura* parametros = malloc(sizeof(parametrosLeitura)); // aloca memoria para os parametros
        parametros->idThread = i; // ID da thread
        parametros->arquivo = strdup(argv[i]); // Arquivo q vai ler
        parametros->vetor = vetorPrincipal; // Ponteiro para o vetor
        
        if (pthread_create(&leitor[i - 1], NULL, leitura, (void*)parametros) != 0) { // i - 1 para o índice da thread pq começa no 0
            printf("Erro ao criar a thread");
            free(parametros->arquivo);
            free(parametros);
            free(vetorPrincipal);
            return 1;
        }
         
    }

    // espera as threads terminarem para continuar
    for (int i = 0; i < argc - 2; i++) {
        pthread_join(leitor[i], NULL);
    }

    // ---------------------------------------------------------- Imprime em um arquivo separado o resultado final, indica q o vetor deu certo
    
    int quebraLinha = 0;
    FILE* saida = fopen(argv[argc-1], "w+");

    for (int i = 0; i < quantidade; i++) { 
        quebraLinha++;
        fprintf(saida, "%d ", vetorPrincipal[i]);

        if (quebraLinha == 25){
            fprintf(saida, "\n");
            quebraLinha = 0;
        }
        
    }

    fclose(saida);
    free(vetorPrincipal);
    
    return 0;
    
}
