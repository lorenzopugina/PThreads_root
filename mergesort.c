#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

//Inspirado no código de exemplo do prof Paulo Feofiloff, disponivel em <https://www.ime.usp.br/~pf/algoritmos/aulas/mrgsrt.html>

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
void imprimeSaida(int* vetor, int tamanho, char* nomeArquivo);

int main(int argc, char const* argv[]){

    int maxThreads = atoi(argv[1]); // ascii to integer
    int maxThreadsCopia = maxThreads;

    sem_t semaforoLeitura;         
    sem_init(&semaforoLeitura, 0, 1);

    // Variaveis utilizadas para cronometrar o tempo
    clock_t inicioTotal;
    clock_t fimTotal;

    int tamanho = 40 * (argc - 4);
    int* vetorPrincipal = (int*)malloc(tamanho * sizeof(int)); // Aloca um vetor inicial de 40 elementos para cada arquivo a ler
    
    pthread_t* threadsLeitores = (pthread_t*)malloc(maxThreads * sizeof(pthread_t)); // Aloca um vetor de threads 
    parametrosLeitura* leitura = malloc(maxThreads * sizeof(parametrosLeitura));  //Aloca um vetor de parametros

    int numElementos = 0;

     for (int j = 2; j < argc-2; j++){ // para cada arquivo a ser lido
        FILE* arquivo = fopen(argv[j], "r");  // arquivo da vez

        for (int i = 0; i < maxThreads; i++) {
            leitura[i].arquivo = arquivo; 
            leitura[i].vetor = &vetorPrincipal;
            leitura[i].numElementos = &numElementos;
            leitura[i].tamanho = &tamanho;
            leitura[i].semaforoLeitura = &semaforoLeitura;
            pthread_create(&threadsLeitores[i], NULL, leitores, &leitura[i]); 
        }

        // Aguarda as threads antes de trocar o arquivo
        for (int i = 0; i < maxThreads; i++) {
            pthread_join(threadsLeitores[i], NULL);
        }

        fclose(arquivo);
    }
    free(threadsLeitores);
    free(leitura);
    
    double* tempoThread = (double*)malloc(maxThreads * sizeof(double)); // Aloca um vetor para salvar os tempos de execucao

    inicioTotal = clock();  

    // Ordena N pedaços do vetor, depois ordena N/2 pedaços 
    while (maxThreads/2 > 0){
        pthread_t* threadsMerge = (pthread_t*)malloc(maxThreads * sizeof(pthread_t)); // Aloca um vetor de threads 
        parametrosMergesort* merge = malloc(maxThreads * sizeof(parametrosMergesort)); // Aloca um vetor de parametros 
        
        int elementosPorThread = numElementos / maxThreads;
        int resto = numElementos % maxThreads;
        int indiceComeço = 0;

        for (int i = 0; i < maxThreads; i++) {
            merge[i].vetor = vetorPrincipal; 
            merge[i].Indicecomeco = indiceComeço;
            merge[i].Indicefinal = indiceComeço + elementosPorThread + (i < resto ? 1 : 0); // Adiciona 1 elemento ao final se i for menor que resto
            merge[i].tempoExecucao = &tempoThread[i];
            
            pthread_create(&threadsMerge[i], NULL, criaMerges, &merge[i]);
            indiceComeço =  merge[i].Indicefinal;
        }

        for (int i = 0; i < maxThreads; i++) {
            pthread_join(threadsMerge[i], NULL);
        }
        
        free(merge);
        free(threadsMerge);

        maxThreads = maxThreads/2;
    } 
    
    pthread_t mergeFInal;
    parametrosMergesort mergesortFinal = {0, numElementos, vetorPrincipal, &tempoThread[0]};
    pthread_create(&mergeFInal, NULL, criaMerges, &mergesortFinal);
    pthread_join(mergeFInal, NULL);

    fimTotal = clock();
    double tempoTotal = (double)(fimTotal - inicioTotal) / CLOCKS_PER_SEC;

    imprimeSaida(vetorPrincipal, numElementos, strdup(argv[argc-1]));

    for (int i = 0; i < maxThreadsCopia; i++){
        printf("Tempo de execução da thread %d foi de: %f\n", i, tempoThread[i]);
    }

    printf("Tempo total de execução foi de: %f\n", tempoTotal);                   

    free(vetorPrincipal);
    free(tempoThread);

    return 0;
}

void* leitores(void* args){
    parametrosLeitura* param = (parametrosLeitura*)args;
    int num;

    while (fscanf(param->arquivo, "%d", &num) == 1) {
        sem_wait(param->semaforoLeitura); // down semaforoLeitura
        
        (*param->vetor)[(*param->numElementos)++] = num;

        // Se for necessario, redimensiona o vetor
        if (*param->numElementos >= *param->tamanho) {
            *param->tamanho += 100;
            *param->vetor = realloc(*param->vetor, *param->tamanho * sizeof(int)); 
        }

       sem_post(param->semaforoLeitura); // up semaforoLeitura
    }
    pthread_exit(NULL);
}

void* criaMerges(void* args) {
    clock_t inicio = clock(); 

    parametrosMergesort* param = (parametrosMergesort*) args;
    int Indicecomeco = param->Indicecomeco;
    int Indicefinal = param->Indicefinal;

    // Enquanto o vetor for maior que 1, será divido ao meio 
    if (Indicecomeco < Indicefinal - 1) {
        int meio = (Indicecomeco + Indicefinal) / 2;
        mergesort(Indicecomeco, meio, param->vetor); 
        mergesort(meio, Indicefinal, param->vetor);  
 
        // Ordena e mescla as metades
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
    while (comeco < meio && metade < Indicefinal){ 

        // Compara os elementos de cada metade, o menor vai pro começo do vetor
        if (vetor[comeco] <= vetor[metade]){          
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

void imprimeSaida(int* vetor, int tamanho, char* nomeArquivo){
    int quebraLinha = 0;

    FILE* saida = fopen(nomeArquivo, "w+");

    for (int i = 0; i < tamanho; i++) { 
        quebraLinha++;
        fprintf(saida, "%d ", vetor[i]);

        if (quebraLinha == 25){
            fprintf(saida, "\n");
            quebraLinha = 0;
        }
    }
    fclose(saida);
    free(nomeArquivo); // Necessário devido ao strdup()
}
