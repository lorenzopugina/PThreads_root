#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    
    int quantidade = 1000;
    
    srand(time(0));

    FILE *arquivo = fopen("arq5.txt", "w+");
    
   
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1; 
    }

    int espaco = 0;
    
    for(int i = 0; i < quantidade; i++) {
        espaco++;
        int numero_aleatorio = rand() % 1001;  
        fprintf(arquivo, "%d ", numero_aleatorio); 

        if (espaco == 25){
           fprintf(arquivo, "\n");
           espaco = 0;
        }
        
    }

    fclose(arquivo);

    return 0;
}