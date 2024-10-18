#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    
    int quantidade = 1000;
    
    srand(time(0));

    FILE *arquivo = fopen("nums3.txt", "w+");
    
   
    if (arquivo == NULL) {
        printf("Num abriu\n");
        return 1; 
    }

    int quebraLinha = 0;
    
    for(int i = 0; i < quantidade; i++) {
        quebraLinha++;
        int numero_aleatorio = rand() % 1001;  
        fprintf(arquivo, "%d ", numero_aleatorio); 

        if (quebraLinha == 25){
           fprintf(arquivo, "\n");
           quebraLinha = 0;
        }
        
    }

    fclose(arquivo);

    return 0;
}