#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main() // argv[1] = num de nucleos, os demais são os arquivos de numeros, argv[argc] = nome do arquivo de saida
{
    FILE* arquivo = fopen("nums1.txt", "r");

    if (fseek(arquivo, (974 * sizeof(int)), SEEK_SET) != 0) {
        perror("Erro ao mover o ponteiro do arquivo");
        fclose(arquivo);
        return 1;
    }

    int num;

    fscanf(arquivo, "%d", &num);
    fclose(arquivo);

    printf("num: %d \n", num);
    return 0;
    
}
