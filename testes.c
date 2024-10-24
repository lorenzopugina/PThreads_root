#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_THREADS 2 // Definir o número máximo de threads

typedef struct {
    int Indicecomeco;
    int Indicefinal;
    int *vetor;
} parametros_mergesort;

void intercala(int Indicecomeco, int meio, int Indicefinal, int vetor[]);
void *criaMerges(void *args);
void mergesort (int Indicecomeco, int Indicefinal, int vetor[]);

int numThread = 0;

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
    pthread_create(&threadInical, NULL, criaMerges, &args);

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

void *criaMerges(void *args) {

    parametros_mergesort *param = (parametros_mergesort *) args;
    int Indicecomeco = param->Indicecomeco;
    int Indicefinal = param->Indicefinal;
    int *vetor = param->vetor;

    if (Indicecomeco < Indicefinal - 1) {
        int meio = (Indicecomeco + Indicefinal) / 2;

        // Armazena os argumentos de cada metade do vetor
        parametros_mergesort arg1 = {Indicecomeco, meio, vetor};
        parametros_mergesort arg2 = {meio, Indicefinal, vetor};

        // Criação de threads para cada metade do vetor
        pthread_t thread1, thread2; 
        int criouThread1 = 0, criouThread2 = 0; // local para cade thread enxergar se deve esperar ou não

        if (numThread < MAX_THREADS - 1) { // cria threads até o limite definido (- 1 pois já tem uma thread quando chega aqui)
            numThread++;
            pthread_create(&thread1, NULL, criaMerges, &arg1);
            criouThread1 = 1;
        } 
        else {
            mergesort(Indicecomeco, meio, vetor); // Se já estiver no limite de threads, continua com a recursividade
        }

        if (numThread < MAX_THREADS - 1) { // mesma coisa só que pra outra metade
            numThread++;
            pthread_create(&thread2, NULL, criaMerges, &arg2);
            criouThread2 = 1; 
        } 
        else {
            mergesort(meio, Indicefinal, vetor);
        }

        // Espera as threads terminarem de dividir o vetor antes de ir para a ordenação
        if (criouThread1) {
            pthread_join(thread1, NULL);
        }
        if (criouThread2) {
            pthread_join(thread2, NULL);
        }

        // ordena e intercala as duas metades
        intercala(Indicecomeco, meio, Indicefinal, vetor);
    }
    numThread--; // Fim da thread, libera para criação de outras
    pthread_exit(NULL);
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

void intercala (int Indicecomeco, int meio, int Indicefinal, int vetor[]) 
{
    int* vetorAUX = (int*)malloc((Indicefinal-Indicecomeco) * sizeof (int));  
    int comeco = Indicecomeco;
    int metade = meio;  
    int contador = 0;  

    while (comeco < meio && metade < Indicefinal) {  // repete o laço até o fim das metades
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
    for (comeco = Indicecomeco; comeco < Indicefinal; ++comeco){ 
        vetor[comeco] = vetorAUX[comeco-Indicecomeco];  
    }  

    free(vetorAUX);  
}


