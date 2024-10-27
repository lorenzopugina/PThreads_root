#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    FILE* arquivo;
    int** vetor; // ponteiro de ponteiro pra dps poder realocar o tamanho
    int* numElementos;
    int* tamanho;
    pthread_mutex_t* mutex;
} parametros;

void* leitores(void* args);
void realocar(int** vetor, int* capacidade); 

int main(int argc, char const *argv[]) {

    int numThreads = atoi(argv[1]); // ascii to integer
    pthread_t* threads = (pthread_t*)malloc(numThreads * sizeof(pthread_t)); // Aloca um vetor de threads
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    int tamanho = 20 * (argc - 3);
    int* vetorPrincipal = (int*)malloc(tamanho * sizeof(int)); // vetor de 20 elementos para cada arquivo a ler
    int numElementos = 0;
 
     for (int j = 2; j < argc-1; j++){ // para cada arquivo a ser lido

        FILE* arquivo = fopen(argv[j], "r");  // arquivo da vez
        if (arquivo == NULL) printf("Não abriu o arquivo"); // tirar dps

        for (int i = 0; i < numThreads; i++) {
            parametros* args = (parametros*)malloc(sizeof(parametros));
            args->arquivo = arquivo; 
            args->vetor = &vetorPrincipal;
            args->numElementos = &numElementos;
            args->tamanho = &tamanho;
            args->mutex = &mutex;
            pthread_create(&threads[i], NULL, leitores, args);
        }

        // Aguardar as threads antes de trocar o arquivo
        for (int i = 0; i < numThreads; i++) {
            pthread_join(threads[i], NULL);
        }

        fclose(arquivo);
    }


    // imprimir no arquivo de saida ------------------- fazer como função dps
    FILE* saida = fopen(argv[argc-1], "w+");
    int quebraLinha = 0;

    for (int i = 0; i < numElementos; i++) { 
        quebraLinha++;
        fprintf(saida, "%d ", vetorPrincipal[i]);

        if (quebraLinha == 25){
            fprintf(saida, "\n");
            quebraLinha = 0;
        }
    }

    fclose(saida);
    free(vetorPrincipal); // nao esquecer dele dps
    free(threads);
    return 0;
}

void* leitores(void* args) {
    parametros* param = (parametros*)args;
    int num;

    while (fscanf(param->arquivo, "%d", &num) == 1) {
        pthread_mutex_lock(param->mutex);
        
        // Verifica se precisa redimensionar o vetor
        if (*param->numElementos >= *param->tamanho) {
            realocar(param->vetor, param->tamanho);
        }

        (*param->vetor)[(*param->numElementos)++] = num;
        pthread_mutex_unlock(param->mutex);
    }
    
    free(param);

    pthread_exit(NULL);
}


void realocar(int** vetor, int* capacidade) {
    *capacidade += 50; //  repensar na matematica
    int* novoVetor = realloc(*vetor, (*capacidade) * sizeof(int));
    
    if (novoVetor == NULL) {          // tirar dps ------------------------------------------
        printf("Erro ao realocar memória\n");
    }

     *vetor = novoVetor; // Atualiza o ponteiro do vetor

}