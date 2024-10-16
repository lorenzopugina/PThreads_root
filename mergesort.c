#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

typedef struct { //parametros que serão utilizados nas threads de leitura
    int idThread;
    char* arquivo;
    int* vetor;
    int indiceComeço;
    int indiceFinal;
    int numArg;
} parametrosLeitura;

sem_t lendoArq; // semaforolendo
long pos; // posição

void* leitura(void* arg){
    
    parametrosLeitura* parametros = (parametrosLeitura*)arg;

    sem_wait(&lendoArq); // down semaforo lendo

    FILE* arquivo = fopen(parametros->arquivo, "r"); // abre o arquivo de cada parametro no modo de leitura binária, pq ta dando B.O o modo normal

    fseek(arquivo, pos, SEEK_SET);

    if (arquivo == NULL){
        printf("Não abriu o arquivo %s\n", parametros->arquivo);
        return NULL;
    }


    for (int i = parametros->indiceComeço; i < parametros->indiceFinal +1; i++) {
        int temp;
        // Lê até 1000 números (ajuste se necessário para seu caso específico)
        if (fscanf(arquivo, "%d", &temp) == 1) {
            parametros->vetor[i+(parametros->numArg * 1000)] = temp;
        } else {
            printf("Thread %d Não leu corretamente\n", parametros->idThread);
            break;
        }

    }

    pos = ftell(arquivo);

    
    fclose(arquivo);
    sem_post(&lendoArq); // up semaforo lendo

    free(parametros->arquivo); // Liberar memória alocada
    free(parametros); // Liberar parâmetros da thread
    pthread_exit(NULL); // Finaliza a thread
}

int main(int argc, char const *argv[]) // argv[1] = num de nucleos, os demais são os arquivos de numeros, argv[argc] = nome do arquivo de saida
{
    
    int numThreads = atoi(argv[1]); // ascii to integer

    sem_init(&lendoArq, 0, 1);

    pthread_t* leitor = (pthread_t*)malloc(numThreads*sizeof(pthread_t)); // Aloca um vetor de threads

    int quantidade = (argc - 3)*1000; // rever em função da parte -o saida.txt
    int* vetorPrincipal = (int*)malloc(quantidade * sizeof(int)); // vetor alocado dinamicamente com o tamanho para cada arquivo q for abrir

    for (int j = 2; j < argc-1; j++){ // argc conta a execução(nome) do arquivo como parametro, porem argv começa no 0 cuidado
        pos = 0;

        for (int i = 0; i < numThreads; i++){

            parametrosLeitura* parametros = malloc(sizeof(parametrosLeitura)); // aloca memoria para os parametros
            parametros->idThread = i; // ID da thread
            parametros->arquivo = strdup(argv[j]); // Arquivo q vai ler
            parametros->vetor = vetorPrincipal; // Ponteiro para o vetor
            parametros->indiceComeço = (1000 / numThreads) * i;
            parametros->indiceFinal = ((1000 / numThreads) * (i + 1)) - 1; // Última thread lê até o fim
            parametros->numArg = j-2;
            
            if (pthread_create(&leitor[i], NULL, leitura, (void*)parametros) != 0) { // i - 1 para o índice da thread pq começa no 0
                printf("Erro ao criar a thread");
                free(parametros->arquivo);
                free(parametros);
                free(vetorPrincipal);
                return 1;
            }
            pthread_join(leitor[i], NULL); // acho q n precisa

        }

        for (int i = 0; i < numThreads; i++) { // esperar terminar antes de trocar de arquivo, precisa por causa da posição
            pthread_join(leitor[i], NULL);
        }
         
    }

    // espera as threads terminarem para continuar
    for (int i = 0; i < numThreads; i++) {
        pthread_join(leitor[i], NULL);
    }

    // ---------------------------------------------------------- Imprime em um arquivo separado o resultado final, indica q o vetor deu certo
    
    int quebraLinha = 0;
    FILE* saida = fopen(argv[argc-1], "w+");

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
