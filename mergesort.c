#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

// #include "funcoes.h"

typedef struct {
    FILE* arquivo;
    int** vetor; 
    int* numElementos;
    int* tamanho;
    sem_t* semaforoLeitura;
} parametrosLeitura;

typedef struct {
    int Indicecomeco;
    int Indicefinal;
    int* vetor;
    sem_t* semaforoMerge;
} parametrosMergesort;

void* leitores(void* args);
void redimensionar(int** vetor, int* capacidade); 
void intercala(int Indicecomeco, int meio, int Indicefinal, int vetor[]);
void* criaMerges(void* args);
void mergesort (int Indicecomeco, int Indicefinal, int vetor[]);
void imprimeSaida(int* vetor, int tamanho, char* arquivo);

int maxThreads;
int numeroAtualThread = 1;
int contador = 0;

int main(int argc, char const* argv[]){

    maxThreads = atoi(argv[1]); // ascii to integer
    pthread_t* threads = (pthread_t*)malloc(maxThreads * sizeof(pthread_t)); // Aloca um vetor de threads   
    if (threads == NULL){
        printf("Alocação para threads falhou\n");
        return 1;
    }
    
    sem_t semaforoLeitura;          // ------------------------------------------------------------------------------- SEMAFORO OU MUTEX?
    sem_init(&semaforoLeitura, 0, 1);

    int tamanho = 20 * (argc - 3);
    int* vetorPrincipal = (int*)malloc(tamanho * sizeof(int)); // Aloca um vetor inicial de 20 elementos para cada arquivo a ler
    if (vetorPrincipal == NULL){
        printf("Alocação inicial do vetor falhou\n");
        return 1;
    }

    int numElementos = 0;
 
     for (int j = 2; j < argc-1; j++){ // para cada arquivo a ser lido

        FILE* arquivo = fopen(argv[j], "r");  // arquivo da vez
        if (arquivo == NULL){
             printf("Não abriu o arquivo para leitura");
             return 1;
        }

        for (int i = 0; i < maxThreads; i++) {
            parametrosLeitura* args = (parametrosLeitura*)malloc(sizeof(parametrosLeitura)); 
            if (args == NULL){
                printf("Alocação para parâmetros de leitura falhou\n");
                return 1;
            }
            
            args->arquivo = arquivo; 
            args->vetor = &vetorPrincipal;
            args->numElementos = &numElementos;
            args->tamanho = &tamanho;
            args->semaforoLeitura = &semaforoLeitura;
            pthread_create(&threads[i], NULL, leitores, args); 
        }

        // Aguardar as threads antes de trocar o arquivo
        for (int i = 0; i < maxThreads; i++) {
            pthread_join(threads[i], NULL);
        }

        fclose(arquivo);
    }

    sem_t semaforoMerge;
    sem_init(&semaforoMerge, 0, 1);

    // Criação da primeira thread merge
    parametrosMergesort args = {0, numElementos, vetorPrincipal, &semaforoMerge}; 
    pthread_t threadInical;

    clock_t inicio = clock(); // aqui -----------------------------------------------------
    pthread_create(&threadInical, NULL, criaMerges, &args); 

    // Espera a thread principal terminar ----------------------------------------------------------------------------------REVER------
    pthread_join(threadInical, NULL);
    clock_t fim = clock();
    double tempoExecucao = (double)(fim - inicio) / CLOCKS_PER_SEC;
    printf("Tempo total de execução: %f segundos\n", tempoExecucao); // aqui ------------------------------------

    sem_destroy(&semaforoMerge); 

    imprimeSaida(vetorPrincipal, numElementos, strdup(argv[argc-1]));

    free(vetorPrincipal);
    free(threads);

    return 0;
}

void* leitores(void* args){
    parametrosLeitura* param = (parametrosLeitura*)args;
    int num;

    while (fscanf(param->arquivo, "%d", &num) == 1) {
        sem_wait(param->semaforoLeitura); // down
        
        // Verifica se precisa redimensionar o vetor
        if (*param->numElementos >= *param->tamanho) {
            redimensionar(param->vetor, param->tamanho);
        }

        (*param->vetor)[(*param->numElementos)++] = num;          
       sem_post(param->semaforoLeitura); // up
    }
    
    free(param); // Necessário pois foi alocada memoria
    pthread_exit(NULL);
}

void redimensionar(int** vetor, int* capacidade){

    *capacidade += 50; 

    int* vetorAux = realloc(*vetor, (*capacidade) * sizeof(int)); 
    if (vetorAux == NULL) {
        printf("Realocação falhou\n");
        return; 
    }

     *vetor = vetorAux; 
}

void* criaMerges(void* args){
    clock_t inicio = clock(); // aqui-----------------------------------------
    contador++;

    parametrosMergesort* param = (parametrosMergesort*) args;
    int Indicecomeco = param->Indicecomeco;
    int Indicefinal = param->Indicefinal;
    int* vetor = param->vetor;

    // Enquanto o vetor for maior que 1, será divido ao meio
    if (Indicecomeco < Indicefinal - 1) { 
        int meio = (Indicecomeco + Indicefinal) / 2; 

        // Criação de threads para cada metade do vetor
        parametrosMergesort arg1 = {Indicecomeco, meio, vetor, param->semaforoMerge};
        parametrosMergesort arg2 = {meio, Indicefinal, vetor, param->semaforoMerge};
        pthread_t thread1, thread2; 
        int criouThread1 = 0, criouThread2 = 0; 

        sem_wait(param->semaforoMerge); // down

        if (numeroAtualThread < maxThreads) { // Cada thread recebe metade do vetor
            numeroAtualThread++;
            sem_post(param->semaforoMerge); // up

            pthread_create(&thread1, NULL, criaMerges, &arg1);
            criouThread1 = 1;
        } else {
            sem_post(param->semaforoMerge); // up
            mergesort(Indicecomeco, meio, vetor); // Se já estiver no limite de threads, continua com a recursividade
        }

        sem_wait(param->semaforoMerge); // down
         // O mesmo processo para a segunda metade do vetor
        if (numeroAtualThread < maxThreads) { 
            numeroAtualThread++;
             sem_post(param->semaforoMerge); // up

            pthread_create(&thread2, NULL, criaMerges, &arg2);
            criouThread2 = 1;
        } else {
             sem_post(param->semaforoMerge); // up
            mergesort(meio, Indicefinal, vetor);
        }

        // Espera as threads terminarem de dividir o vetor antes de ir para a ordenação
        if (criouThread1) {
            pthread_join(thread1, NULL);
        }
        if (criouThread2) {
            pthread_join(thread2, NULL);
        }

        // Ordena e mescla as metades
        intercala(Indicecomeco, meio, Indicefinal, vetor);
    }
    
    clock_t fim = clock(); // ---------------------------------------
    double tempoExecucao = (double)(fim - inicio) / CLOCKS_PER_SEC;
    printf("Tempo de execução da thread %d: %f segundos\n", contador, tempoExecucao);

    pthread_exit(NULL);
}


void mergesort(int Indicecomeco, int Indicefinal, int vetor[]){

    // Enquanto o vetor for maior que 1, será divido ao meio  
    if (Indicecomeco < (Indicefinal-1)) {        
      int meio = (Indicecomeco + Indicefinal)/2; 
      mergesort (Indicecomeco, meio, vetor);     
      mergesort (meio, Indicefinal, vetor);  

      // Ordena e mescla as metades
      intercala (Indicecomeco, meio, Indicefinal, vetor); 
    }
}

void intercala(int Indicecomeco, int meio, int Indicefinal, int vetor[]){

    int* vetorAux = (int*)malloc((Indicefinal-Indicecomeco) * sizeof (int));  
    if (vetorAux == NULL) {
        printf("Alocação para vetor auxiliar falhou\n");
        return; 
    }
    
    int comeco = Indicecomeco;
    int metade = meio;  
    int contador = 0;  

    // Repete o laço até o fim das metades
    while (comeco < meio && metade < Indicefinal) { 

        if (vetor[comeco] <= vetor[metade]){          // Compara os elementos de cada metade, o menor vai pro começo do vetor
              vetorAux[contador++] = vetor[comeco++];  
        }
        else  vetorAux[contador++] = vetor[metade++];  
    }  

    // Copia os elementos que sobraram. Como estão no final do vetor, são os maiores
    while (comeco < meio){
          vetorAux[contador++] = vetor[comeco++]; 
    }
    while (metade < Indicefinal){
          vetorAux[contador++] = vetor[metade++]; 
    }

    // Copia para o vetor principal
    for (comeco = Indicecomeco; comeco < Indicefinal; ++comeco){ 
        vetor[comeco] = vetorAux[comeco-Indicecomeco];  
    }  

    free(vetorAux);  
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
    free(arquivo); // Necessário devido ao strdup()
}