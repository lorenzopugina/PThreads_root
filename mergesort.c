#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

int main(int argc, char const *argv[]) // argv[1] = num de nucleos, os demais são os arquivos de numeros, argv[argc] = nome do arquivo de saida
{
    int quantidade = (argc - 1)*1000; // rever em função da parte -o saida.txt
    int indiceVetor = 0;

    int* vetorPrincipal = (int*)malloc(quantidade * sizeof(int)); // vetor alocado dinamicamente com o tamanho para cada arquivo q for abrir

    for (int i = 1; i < argc-1; i++){ // argc conta a execução(nome) do arquivo como parametro, porem argv começa no 0 cuidado

        FILE* arquivo = fopen(argv[i], "r"); // abre o arquivo de cada parametro no modo de leitura

        if (arquivo == NULL){
            printf("Não abriu o arquivo %s\n", argv[i]);
            return 1;
        }
        
        while (fscanf(arquivo, "%d", &vetorPrincipal[indiceVetor]) != EOF){ // Salva na matriz os numeros do arquivo
            indiceVetor++;
        }
        fclose(arquivo);
    }
    
}
