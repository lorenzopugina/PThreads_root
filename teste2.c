#include <stdio.h>
#include <stdlib.h>

//https://www.ime.usp.br/~pf/algoritmos/aulas/mrgsrt.html#mergesort

void intercala (int Indicecomeco, int meio, int Indicefinal, int vetor[]) 
{
    int* vetorAUX = (int*)malloc((Indicefinal-Indicecomeco) * sizeof (int));  
    int comeco = Indicecomeco;
    int metade = meio;  
    int contador = 0;  

    while (comeco < meio && metade < Indicefinal) {  
        if (vetor[comeco] <= vetor[metade]){
              vetorAUX[contador++] = vetor[comeco++];  
        }
        else  vetorAUX[contador++] = vetor[metade++];  
    }  

    while (comeco < meio){
          vetorAUX[contador++] = vetor[comeco++]; 
    }

    while (metade < Indicefinal){
          vetorAUX[contador++] = vetor[metade++]; 
    }

    for (comeco = Indicecomeco; comeco < Indicefinal; ++comeco){//adiciona primeiro
        vetor[comeco] = vetorAUX[comeco-Indicecomeco];  
    }  

    free(vetorAUX);  
}

void mergesort (int Indicecomeco, int Indicefinal, int vetor[])
{
   if (Indicecomeco < Indicefinal-1) {  // compara o começo com o final do vetor, enquanto forem diff, são cortados ao meio          
      int meio = (Indicecomeco + Indicefinal)/2; // o valor será truncado se impar     
      mergesort (Indicecomeco, meio, vetor); //pega do começo ao meio      
      mergesort (meio, Indicefinal, vetor);  //pega do meio ao final   
      intercala (Indicecomeco, meio, Indicefinal, vetor);     
   }
}

int main()
{
    int v[7]={15, 8, 9, 3, 1, 4, 4};

    mergesort(0, 7, v); // 0 é o começo, 6 é o numElementos, v é o vetor

    for (int i = 0; i < 7; i++){
        printf("%d ", v[i]);
    }
    printf("\n");
    
    return 0;
}
