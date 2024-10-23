#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_THREADS 2 // Definir o número máximo de threads

typedef struct {
    int Indicecomeco;
    int Indicefinal;
    int *vetor;
} parametros_mergesort;

int numThread = 0;

void intercala(int Indicecomeco, int meio, int Indicefinal, int vetor[]);
void *mergesort(void *args);

int main() {

    FILE* arquivo = fopen("teste.txt", "r");
    int n = 10;

    int* vetor = (int*)malloc(n * sizeof(int));

    for (int i = 0; i < n; i++){
        fscanf(arquivo, "%d", &vetor[i]);
    }
    
    fclose(arquivo);

    parametros_mergesort args = {0, n, vetor};

    // Criação da primeira thread
    pthread_t threadInical;
    pthread_create(&threadInical, NULL, mergesort, &args);

    // Espera a thread principal terminar
    pthread_join(threadInical, NULL);

    // --------------------------------------------------------Imprime o vetor ordenado
    for (int i = 0; i < n; i++) {
        printf("%d ", vetor[i]);
    }
    printf("\n");

    free(vetor);

    return 0;
}

void *mergesort(void *args) {

    parametros_mergesort *param = (parametros_mergesort *) args;
    int Indicecomeco = param->Indicecomeco;
    int Indicefinal = param->Indicefinal;
    int *vetor = param->vetor;

    if (Indicecomeco < Indicefinal - 1) {
        int meio = (Indicecomeco + Indicefinal) / 2;

        // Armazena os argumentos de cada metade do vetor
        parametros_mergesort arg1 = {Indicecomeco, meio, vetor};
        parametros_mergesort arg2 = {meio, Indicefinal, vetor};

        // Criação de threads para cada metade
        pthread_t thread1, thread2;

        if (numThread < MAX_THREADS) { // cria threads até o limite definido
            pthread_create(&thread1, NULL, mergesort, &arg1);
            numThread++;
            pthread_join(thread1, NULL); // tira isso dps ------------------------------------------------
        } 
        else {
            mergesort(&arg1); // Se ja estiver no limite, continua com a recursividade
        }

        if (numThread < MAX_THREADS) { // mesma coisa só q pra outra metade
            pthread_create(&thread2, NULL, mergesort, &arg2);
            numThread++;
            pthread_join(thread2, NULL); // tira isso deps ----------------------------------------------
        } 
        else {
            mergesort(&arg2);
        }

        // Espera as threads terminarem

        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);
        

        // Intercala as duas metades
        intercala(Indicecomeco, meio, Indicefinal, vetor);
    }

    numThread--;
    pthread_exit(NULL);
}

void intercala (int Indicecomeco, int meio, int Indicefinal, int vetor[]) 
{
    int* vetorAUX = (int*)malloc((Indicefinal-Indicecomeco) * sizeof (int));  
    int comeco = Indicecomeco;
    int metade = meio;  
    int contador = 0;  

    while (comeco < meio && metade < Indicefinal) {  
        if (vetor[comeco] <= vetor[metade]){          
              vetorAUX[contador++] = vetor[comeco++];  // Se o elemento da primeira parte do vetor for menor, ele vai pro começo
        }
        else  vetorAUX[contador++] = vetor[metade++];  // Se n é o elemento da segunda metade
    }  

    // Pode sobrar elementos, como estão no final do vetor eles ja são maiores, portanto só são copiados
    while (comeco < meio){
          vetorAUX[contador++] = vetor[comeco++]; 
    }

    while (metade < Indicefinal){
          vetorAUX[contador++] = vetor[metade++]; 
    }

    // Copia para o vetor principal
    for (comeco = Indicecomeco; comeco < Indicefinal; ++comeco){ //adiciona primeiro
        vetor[comeco] = vetorAUX[comeco-Indicecomeco];  
    }  

    free(vetorAUX);  
}