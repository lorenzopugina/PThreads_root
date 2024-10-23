#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_THREADS 4  // Definir o número máximo de threads

typedef struct {
    int Indicecomeco;
    int Indicefinal;
    int *vetor;
} parametros_mergesort;

void intercala(int Indicecomeco, int meio, int Indicefinal, int vetor[])
{
    int *vetorAUX = malloc((Indicefinal - Indicecomeco) * sizeof(int));
    int comeco = Indicecomeco, metade = meio, contador = 0;

    while (comeco < meio && metade < Indicefinal) {
        if (vetor[comeco] <= vetor[metade])
            vetorAUX[contador++] = vetor[comeco++];
        else
            vetorAUX[contador++] = vetor[metade++];
    }
    while (comeco < meio)
        vetorAUX[contador++] = vetor[comeco++];
    while (metade < Indicefinal)
        vetorAUX[contador++] = vetor[metade++];

    for (comeco = Indicecomeco; comeco < Indicefinal; ++comeco)
        vetor[comeco] = vetorAUX[comeco - Indicecomeco];

    free(vetorAUX);
}

void *mergesort(void *args) {
    parametros_mergesort *param = (parametros_mergesort *) args;
    int Indicecomeco = param->Indicecomeco;
    int Indicefinal = param->Indicefinal;
    int *vetor = param->vetor;

    if (Indicecomeco < Indicefinal - 1) {
        int meio = (Indicecomeco + Indicefinal) / 2;

        // Estrutura para armazenar os parâmetros das threads
        parametros_mergesort arg1 = {Indicecomeco, meio, vetor};
        parametros_mergesort arg2 = {meio, Indicefinal, vetor};

        // Criação das threads
        pthread_t thread1, thread2;
        int num_threads = 0;

        if (num_threads < MAX_THREADS) {
            pthread_create(&thread1, NULL, mergesort, &arg1);
            num_threads++;
        } else {
            mergesort(&arg1); // Se não puder criar mais threads, chama recursivamente
        }

        if (num_threads < MAX_THREADS) {
            pthread_create(&thread2, NULL, mergesort, &arg2);
            num_threads++;
        } else {
            mergesort(&arg2);
        }

        // Esperar as threads terminarem
        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);

        // Intercala as duas metades
        intercala(Indicecomeco, meio, Indicefinal, vetor);
    }
    return NULL;
}

int main() {
    int v[] = {15, 8, 9, 3, 1, 4, 7, 6, 10, 2, 12, 11, 5};
    int n = sizeof(v) / sizeof(v[0]);

    parametros_mergesort args = {0, n, v};

    // Criação da primeira thread
    pthread_t initial_thread;
    pthread_create(&initial_thread, NULL, mergesort, &args);

    // Espera a thread principal terminar
    pthread_join(initial_thread, NULL);

    // Imprime o vetor ordenado
    for (int i = 0; i < n; i++) {
        printf("%d ", v[i]);
    }
    printf("\n");

    return 0;
}
