#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// #include "funcoes.h"

typedef struct {
    FILE* arquivo;
    int** vetor; 
    int* numElementos;
    int* tamanho;
    pthread_mutex_t* mutex;
} parametrosLeitura;

typedef struct {
    int Indicecomeco;
    int Indicefinal;
    int* vetor;
    int numThread;
} parametrosMergesort;

void* leitores(void* args);
void redimensionar(int** vetor, int* capacidade); 
void intercala(int Indicecomeco, int meio, int Indicefinal, int vetor[]);
void* criaMerges(void* args);
void mergesort (int Indicecomeco, int Indicefinal, int vetor[]);
void imprimeSaida(int* vetor, int tamanho, char* arquivo);

int maxThreads;

int main(int argc, char const* argv[]) {

    maxThreads = atoi(argv[1]); // ascii to integer
    pthread_t* threads = (pthread_t*)malloc(maxThreads * sizeof(pthread_t)); // Aloca um vetor de threads   ----------------------- VERIFICA NULL?
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    int tamanho = 20 * (argc - 3);
    int* vetorPrincipal = (int*)malloc(tamanho * sizeof(int)); // Aloc um vetor inicial de 20 elementos para cada arquivo a ler ----------- NULL?
    int numElementos = 0;
 
     for (int j = 2; j < argc-1; j++){ // para cada arquivo a ser lido

        FILE* arquivo = fopen(argv[j], "r");  // arquivo da vez
        if (arquivo == NULL){
             printf("Não abriu o arquivo para leitura");
             return 1;
        }

        for (int i = 0; i < maxThreads; i++) {
            parametrosLeitura* args = (parametrosLeitura*)malloc(sizeof(parametrosLeitura)); // ----------------------------VERIFICA NULL?
            args->arquivo = arquivo; 
            args->vetor = &vetorPrincipal;
            args->numElementos = &numElementos;
            args->tamanho = &tamanho;
            args->mutex = &mutex;
            pthread_create(&threads[i], NULL, leitores, args);
        }

        // Aguardar as threads antes de trocar o arquivo
        for (int i = 0; i < maxThreads; i++) {
            pthread_join(threads[i], NULL);
        }

        fclose(arquivo);
    }

    // Criação da primeira thread merge
    parametrosMergesort args = {0, numElementos, vetorPrincipal, 1}; // 1 pois será a primeira thread
    pthread_t threadInical;
    pthread_create(&threadInical, NULL, criaMerges, &args);

    // Espera a thread principal terminar ----------------------------------------------------------------------------------REVER------
    pthread_join(threadInical, NULL);

    imprimeSaida(vetorPrincipal, numElementos, strdup(argv[argc-1]));

    free(vetorPrincipal);
    free(threads);

    return 0;
}

void* leitores(void* args) {
    parametrosLeitura* param = (parametrosLeitura*)args;
    int num;

    while (fscanf(param->arquivo, "%d", &num) == 1) {
        pthread_mutex_lock(param->mutex); // ---------------------------------------------------------------------------------- REVER
        
        // Verifica se precisa redimensionar o vetor
        if (*param->numElementos >= *param->tamanho) {
            redimensionar(param->vetor, param->tamanho);
        }

        (*param->vetor)[(*param->numElementos)++] = num;
        pthread_mutex_unlock(param->mutex);
    }
    
    free(param); // necessário pois foi alocada memoria
    pthread_exit(NULL);
}

void redimensionar(int** vetor, int* capacidade) {
    *capacidade += 50; 

    int* vetorAux = realloc(*vetor, (*capacidade) * sizeof(int)); 
    if (vetorAux == NULL) {
        printf("Realocação falhou\n");
        return; 
    }

     *vetor = vetorAux; 
}

void *criaMerges(void* args) {

    parametrosMergesort* param = (parametrosMergesort*) args;
    int Indicecomeco = param->Indicecomeco;
    int Indicefinal = param->Indicefinal;
    int* vetor = param->vetor;

    // Enquanto o vetor for maior que 1, será divido ao meio
    if (Indicecomeco < Indicefinal - 1) { 
        int meio = (Indicecomeco + Indicefinal) / 2; 


        parametrosMergesort arg1 = {Indicecomeco, meio, vetor, param->numThread};
        parametrosMergesort arg2 = {meio, Indicefinal, vetor, param->numThread};

        // Criação de threads para cada metade do vetor
        pthread_t thread1, thread2; 
        int criouThread1 = 0, criouThread2 = 0; 


        if (param->numThread < maxThreads) { // cada thread recebe metade do vetor
            arg1.numThread++;               // -------------------------------------------------------------------------------MUTEX?
            arg2.numThread++; 
            pthread_create(&thread1, NULL, criaMerges, &arg1);
            criouThread1 = 1;
        } else {
            mergesort(Indicecomeco, meio, vetor); // Se já estiver no limite de threads, continua com a recursividade
        }

         // O mesmo processo para a segunda metade do vetor
        if (param->numThread < maxThreads) { 
            arg1.numThread++;               // -------------------------------------------------------------------------------MUTEX?
            arg2.numThread++; 
            pthread_create(&thread2, NULL, criaMerges, &arg2);
            criouThread2 = 1;
        } else {
            mergesort(meio, Indicefinal, vetor);
        }

        // Espera as threads terminarem de dividir o vetor antes de ir para a ordenação
        if (criouThread1) {
            pthread_join(thread1, NULL);
        }
        if (criouThread2) {
            pthread_join(thread2, NULL);
        }

        // ordena e mescla as metades
        intercala(Indicecomeco, meio, Indicefinal, vetor);

        if (param->numThread > 0){
            param->numThread--;    // -----------------------------------------------------------------------------------------MUTEX?
        }
    }
     
    pthread_exit(NULL);
}


void mergesort (int Indicecomeco, int Indicefinal, int vetor[]){

    // Enquanto o vetor for maior que 1, será divido ao meio  
    if (Indicecomeco < (Indicefinal-1)) {        
      int meio = (Indicecomeco + Indicefinal)/2; 
      mergesort (Indicecomeco, meio, vetor);     
      mergesort (meio, Indicefinal, vetor);  

      // ordena e mescla as metades
      intercala (Indicecomeco, meio, Indicefinal, vetor); 
    }
}

void intercala (int Indicecomeco, int meio, int Indicefinal, int vetor[]) 
{
    int* vetorAUX = (int*)malloc((Indicefinal-Indicecomeco) * sizeof (int));  // -------------------------------------------------- NULL?
    int comeco = Indicecomeco;
    int metade = meio;  
    int contador = 0;  

    // repete o laço até o fim das metades
    while (comeco < meio && metade < Indicefinal) { 

        if (vetor[comeco] <= vetor[metade]){          // Compara os elementos de cada metade, o menor vai pro começo do vetor
              vetorAUX[contador++] = vetor[comeco++];  
        }
        else  vetorAUX[contador++] = vetor[metade++];  
    }  

    // Copia os elementos que sobraram. Como estão no final do vetor, são os maiores
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

void imprimeSaida(int* vetor, int tamanho, char* arquivo){
    int quebraLinha = 0;

    FILE* saida = fopen(arquivo, "w+");
    if (saida == NULL){
        printf("Não abriu o arquivo para escrita");
        free(arquivo);
        return;
    }

    for (int i = 0; i < tamanho; i++) { 
        quebraLinha++;
        fprintf(saida, "%d ", vetor[i]);

        if (quebraLinha == 25){
            fprintf(saida, "\n");
            quebraLinha = 0;
        }
    }

    fclose(saida);
    free(arquivo);
}