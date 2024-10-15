#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) // argc = numero de parametros passados na execução. argv[] os parametros ordenados no vetor
{
    int numLido[argc]; 

    for (int i = 1; i <= argc; i++){ // começa com 1 pq argc[0] é o nome do arquivo
        FILE* arquivo = fopen(argv[i], "r");

        if (arquivo == NULL){
            continue;
        }
        
        fscanf(arquivo, "%d", &numLido[i]); // le o primeiro numero do arquivo e salva num vetor
        fclose(arquivo);
    }

    FILE* saida = fopen("saida.txt", "w+");//w+ = se n tiver, cria o arquivo

    for (int i = 1; i < argc; i++){
        fprintf(saida, "%d ", numLido[i]);// imprime o primeiro numero de cada arquivo digitado
    }

    fclose(saida);
    
    return 0;
}
