#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

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
    double* tempoExecucao;
} parametrosMergesort;

void* leitores(void* args);
void intercala(int Indicecomeco, int meio, int Indicefinal, int* vetor);
void* criaMerges(void* args);
void mergesort (int Indicecomeco, int Indicefinal, int* vetor);
void imprimeSaida(int* vetor, int tamanho, char* arquivo);

int main(int argc, char const* argv[]){

    if (argc == 1){
        return 0;
    }
    
    int maxThreads = atoi(argv[1]); // ascii to integer
    int maxThreadsCopia = maxThreads;

    sem_t semaforoLeitura;         
    sem_init(&semaforoLeitura, 0, 1);

    int tamanho = 20 * (argc - 4);
    int* vetorPrincipal = (int*)malloc(tamanho * sizeof(int)); // Aloca um vetor inicial de 20 elementos para cada arquivo a ler
    
    pthread_t* threads = (pthread_t*)malloc(maxThreads * sizeof(pthread_t)); // Aloca um vetor de threads 
    parametrosLeitura* leitura = malloc(maxThreads * sizeof(parametrosLeitura));  

    int numElementos = 0;

     for (int j = 2; j < argc-2; j++){ // para cada arquivo a ser lido
        FILE* arquivo = fopen(argv[j], "r");  // arquivo da vez

        for (int i = 0; i < maxThreads; i++) {
            leitura[i].arquivo = arquivo; 
            leitura[i].vetor = &vetorPrincipal;
            leitura[i].numElementos = &numElementos;
            leitura[i].tamanho = &tamanho;
            leitura[i].semaforoLeitura = &semaforoLeitura;
            pthread_create(&threads[i], NULL, leitores, &leitura[i]); 
        }

        // Aguardar as threads antes de trocar o arquivo
        for (int i = 0; i < maxThreads; i++) {
            pthread_join(threads[i], NULL);
        }

        fclose(arquivo);
    }
    free(threads);
    free(leitura);
    
    double* tempoExecucao = (double*)malloc(maxThreads * sizeof(double));

    while (maxThreads/2 > 0){
        pthread_t* threadsMerge = (pthread_t*)malloc(maxThreads * sizeof(pthread_t)); // Aloca um vetor de threads 
        parametrosMergesort* merge = malloc(maxThreads * sizeof(parametrosMergesort));
        
        int elementosPorThread = numElementos / maxThreads;
        int resto = numElementos % maxThreads;
        int indiceComeço = 0;

        for (int i = 0; i < maxThreads; i++) {
            merge[i].vetor = vetorPrincipal; 
            merge[i].Indicefinal = indiceComeço + elementosPorThread + (i < resto ? 1 : 0); // Distribui o resto
            merge[i].Indicecomeco = indiceComeço;
            merge[i].tempoExecucao = &tempoExecucao[i];
            //-----------------------------------------------------------------------------------------------------------------------Cronometro?
            pthread_create(&threadsMerge[i], NULL, criaMerges, &merge[i]);
            indiceComeço =  merge[i].Indicefinal;
        }

        for (int i = 0; i < maxThreads; i++) {
            pthread_join(threadsMerge[i], NULL);
        }
        //--------------------------------------------------------------------------------------------------------------------- fim cornometro?
        for (int i = 0; i < maxThreads; i++){
            tempoExecucao[i] = *merge[i].tempoExecucao; // salva os tempos de execução
        }
        
        free(merge);
        free(threadsMerge);

        maxThreads = maxThreads/2;
    } 
    
    pthread_t mergeFInal;
    parametrosMergesort mergesortFinal = {0, numElementos, vetorPrincipal, &tempoExecucao[0]};
    pthread_create(&mergeFInal, NULL, criaMerges, &mergesortFinal);

    imprimeSaida(vetorPrincipal, numElementos, strdup(argv[argc-1]));

    for (int i = 0; i < maxThreadsCopia; i++){
        printf("Tempo total de execução da thread %d foi de: %f\n", i, tempoExecucao[i]);
    }

    //---------------------------------------------------------Imprimir tempo total
    free(vetorPrincipal);


    return 0;
}

void* leitores(void* args){
    parametrosLeitura* param = (parametrosLeitura*)args;
    int num;

    while (fscanf(param->arquivo, "%d", &num) == 1) {
        sem_wait(param->semaforoLeitura); // down
        
        // Verifica se precisa redimensionar o vetor
        if (*param->numElementos >= *param->tamanho) {
            *param->vetor = realloc(*param->vetor, (*param->tamanho += 50) * sizeof(int)); //------------------------------------ CONFUSO?
        }

        (*param->vetor)[(*param->numElementos)++] = num;          
       sem_post(param->semaforoLeitura); // up
    }
    pthread_exit(NULL);
}

void* criaMerges(void* args) {
    clock_t inicio = clock(); 

    parametrosMergesort* param = (parametrosMergesort*) args;
    int Indicecomeco = param->Indicecomeco;
    int Indicefinal = param->Indicefinal;

    if (Indicecomeco < Indicefinal - 1) {
        int meio = (Indicecomeco + Indicefinal) / 2;
        mergesort(Indicecomeco, meio, param->vetor);  //------------------------------------------AUto recursividade funciona mas com warning
        mergesort(meio, Indicefinal, param->vetor);  
 
        intercala(Indicecomeco, meio, Indicefinal, param->vetor);
    }

    clock_t fim = clock();
    *param->tempoExecucao = (double)(fim - inicio) / CLOCKS_PER_SEC;

    pthread_exit(NULL);
}

void mergesort(int Indicecomeco, int Indicefinal, int* vetor){

    // Enquanto o vetor for maior que 1, será divido ao meio  
    if (Indicecomeco < Indicefinal - 1) {        
      int meio = (Indicecomeco + Indicefinal)/2; 
      mergesort (Indicecomeco, meio, vetor);     
      mergesort (meio, Indicefinal, vetor);  

      // Ordena e mescla as metades
      intercala (Indicecomeco, meio, Indicefinal, vetor); 
    }
}

void intercala(int Indicecomeco, int meio, int Indicefinal, int* vetor){

    int* vetorAux = (int*)malloc((Indicefinal-Indicecomeco) * sizeof (int));  
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