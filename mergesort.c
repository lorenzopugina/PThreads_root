#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct { //parametros que serão utilizados nas threads de leitura
    int idThread;
    char* arquivo;
    int* vetor;
    int indiceComeço;
    int indiceFinal;
    int numArquivo;
} parametrosLeitura;

long pos; // posição do ponteiro de leitura
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int vez = 0; // vez de acesso decada thread


void* leitura(void* arg);

int main(int argc, char const *argv[]) // argv[1] = num de nucleos, os demais são os arquivos de numeros, argv[argc] = nome do arquivo de saida
{
    int numThreads = atoi(argv[1]); // ascii to integer
    pthread_t* leitor = (pthread_t*)malloc(numThreads*sizeof(pthread_t)); // Aloca um vetor de threads

    int quantidade = (argc - 3)*1000; // rever em função da parte -o saida.txt
    int* vetorPrincipal = (int*)malloc(quantidade * sizeof(int)); // vetor alocado dinamicamente com o tamanho para cada arquivo q for abrir

    for (int j = 2; j < argc-1; j++){ // argc conta a execução(nome) do arquivo como parametro, porem argv começa no 0 cuidado
        pos = 0;
        vez = 0;

        for (int i = 0; i < numThreads; i++){
            parametrosLeitura* parametros = malloc(sizeof(parametrosLeitura)); // aloca memoria para os parametros
            if (parametros == NULL) {
                fprintf(stderr, "Erro ao alocar memória para parâmetros.\n");
                return 1;
            }
            parametros->idThread = i; // ID da thread
            parametros->arquivo = strdup(argv[j]); // Arquivo q vai ler
            parametros->vetor = vetorPrincipal; // Ponteiro para o vetor
            parametros->indiceComeço = (1000 / numThreads) * i;
            parametros->indiceFinal = ((1000 / numThreads) * (i + 1)) - 1; // Última thread lê até o fim
            parametros->indiceFinal = (i == numThreads - 1) ? (quantidade - 1) : (((1000 / numThreads) * (i + 1)) - 1);

            parametros->numArquivo = j-2;
            
            if (pthread_create(&leitor[i], NULL, leitura, (void*)parametros) != 0) { // i - 1 para o índice da thread pq começa no 0
                printf("Erro ao criar a thread");
                free(parametros->arquivo);
                free(parametros);
                free(vetorPrincipal);
                return 1;
            }
        }
            
        for (int i = 0; i < numThreads; i++) { // esperar terminar antes de trocar de arquivo, precisa por causa da posição
            pthread_join(leitor[i], NULL);
        }
    }

    // ---------------------------------------------------------- Imprime em um arquivo separado o resultado final, indica q o vetor deu certo

    FILE* saida = fopen(argv[argc-1], "w+");

    int quebraLinha = 0;
    for (int i = 0; i < quantidade; i++) { 
        quebraLinha++;
        fprintf(saida, "%d ", vetorPrincipal[i]);

        if (quebraLinha == 25){
            fprintf(saida, "\n");
            quebraLinha = 0;
        }
        
    }

    fclose(saida);
    free(vetorPrincipal);
    
    return 0;
}

void* leitura(void* arg){

    parametrosLeitura* parametros = (parametrosLeitura*)arg;

    pthread_mutex_lock(&mutex);
    while (parametros->idThread != vez){ // aguarda sua vez
        pthread_cond_wait(&cond, &mutex);
    }

    FILE* arquivo = fopen(parametros->arquivo, "r"); // abre o arquivo de cada parametro no modo de leitura binária, pq ta dando B.O o modo normal
    if (arquivo == NULL){
        printf("Não abriu o arquivo %s\n", parametros->arquivo);
        pthread_cond_broadcast(&cond); // acorda a thread para verificarem sua condição
        pthread_mutex_unlock(&mutex);

        free(parametros->arquivo); // Liberar memória alocada
        free(parametros); // Liberar parâmetros da thread
        pthread_exit(NULL); // Finaliza a thread
        return NULL;
    }

    fseek(arquivo, pos, SEEK_SET);

    for (int i = parametros->indiceComeço; i < parametros->indiceFinal +1; i++) {
        int temp;
        // Lê até 1000 números (ajuste se necessário para seu caso específico)
        if (fscanf(arquivo, "%d", &temp) == 1) {
            parametros->vetor[i+(parametros->numArquivo * 1000)] = temp;
        } else {
            printf("Thread %d Não leu corretamente\n", parametros->idThread);
            printf("Posicao requeria do vetor: %d\n", (i+(parametros->numArquivo * 1000)));
            break;
        }

    }

    pos = ftell(arquivo); // guarda a posição do ponteiro 
    fclose(arquivo);
    vez++;
    pthread_cond_broadcast(&cond); // acorda a thread para verificarem sua condição
    pthread_mutex_unlock(&mutex);

    free(parametros->arquivo); // Liberar memória alocada
    free(parametros); // Liberar parâmetros da thread
    pthread_exit(NULL); // Finaliza a thread
}