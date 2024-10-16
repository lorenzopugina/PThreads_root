#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

int main(int argc, char const *argv[]) // argv[1] = num de nucleos, os demais são os arquivos de numeros, argv[argc] = nome do arquivo de saida
{
    FILE* arquivo = fopen("teste.txt", "r");


    for (int i = 0; i < 20; i++)
    {
       int num;
        fscanf(arquivo, "%d", &num);
        printf("LIdo\n");
        long pos = ftell(arquivo);
        printf("Posição atual do ponteiro: %ld\n", pos);
    }

    
    
    return 0;
    
}
