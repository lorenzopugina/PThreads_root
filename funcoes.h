#ifndef FUNCOES_H
#define FUNCOES_H

#include <stdio.h>      
#include <pthread.h> 

typedef struct {
    FILE* arquivo;
    int** vetor; // ponteiro de ponteiro pra dps poder realocar o tamanho
    int* numElementos;
    int* tamanho;
    pthread_mutex_t* mutex;
} parametrosLeitura;

typedef struct {
    int Indicecomeco;
    int Indicefinal;
    int* vetor;
    int numThread
} parametrosMergesort;

void* leitores(void* args);
void realocar(int** vetor, int* capacidade); 
void intercala(int Indicecomeco, int meio, int Indicefinal, int vetor[]);
void* criaMerges(void* args);
void mergesort (int Indicecomeco, int Indicefinal, int vetor[]);

int maxThreads;

#endif 