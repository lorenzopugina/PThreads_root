/*Solucao da 10a questao da lista de exercicios, 
 * que simula as regras de um banheiro.
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define PESSOAS 10 // Define a quantidade total de pessoas

/*Declaração dos semaforos*/
sem_t mutex_num_mulheres; // Semaforo para regular o acesso das mulheres
sem_t mutex_num_homens; // Semaforo para regular o acesso dos homens
sem_t banheiro; // Semaforo para regiao critica (banheiro)

int num_Mulher = 0; // numero de mulheres no banheiro.
int num_Homem = 0; // numero de homens no banheiro.

/*Funcao que implementa a entrada de uma mulher no banheiro*/
void mulher_entra(unsigned int id)
{
  printf("Mulher %d esta dentro do banheiro <!>\n",id);
  sleep(rand()%3); /* Aguarda um tempo aleatorio (1, 2 ou 3s)*/ 
  return;
}  

/*Funcao que implementa a entrada de um homem no banheiro*/
void homem_entra(unsigned int id)
{
  printf("Homem %d esta dentro do banheiro <!>\n",id);
  sleep(rand()%3); /* Aguarda um tempo aleatorio (1, 2 ou 3s)*/
  return;
}

/*Funcao que implementa a saida de uma mulher no banheiro*/
void mulher_sai(unsigned int id)
{
  printf("Mulher %d esta saindo do banheiro <out>.\n",id);
  return;
}

/*Funcao que implementa a saida de um homem no banheiro*/
void homem_sai(unsigned int id)
{
  printf("Homem %d esta saindo do banheiro <out>.\n",id);
  return;
}

/*Thread que implementa a tentativa da entrada de uma mulher no banheiro*/
void *Mulher_quer_entrar(void *idM)
{
  unsigned int id;

  id =  *((unsigned int *)idM);

  printf("Mulher %d esta querendo entrar no banheiro (->).\n",id);

  sem_wait(&mutex_num_mulheres); //Down(mutex_num_mulheres)
    num_Mulher++;

    if(num_Mulher == 1)
      sem_wait(&banheiro); // Down(banheiro)

  sem_post(&mutex_num_mulheres); // Up(mutex_num_mulheres)

  mulher_entra(id);

  sem_wait(&mutex_num_mulheres); // Down(mutex_num_mulheres)
     num_Mulher--;
     mulher_sai(id);
     if (num_Mulher == 0)
     {
         sem_post(&banheiro); //Up(banheiro)
         puts("Banheiro livre");
     }
  sem_post(&mutex_num_mulheres); //Down(mutex_num_mulheres)
  free(idM);

  pthread_exit(NULL);
}

/*Thread que implementa a tentativa da entrada de um homem no banheiro*/
void *Homem_quer_entrar(void *idH)
{
  int id;

  id = *((unsigned int *) idH);
  printf("Homem %d esta querendo entrar no banheiro (->).\n", id);


  sem_wait(&mutex_num_homens); //Down(mutex_num_homens)
    num_Homem++;

    if (num_Homem == 1)
        sem_wait(&banheiro); //Down(banheiro)

  sem_post(&mutex_num_homens); //Up(mutex_num_homens)

  homem_entra(id);
  
  sem_wait(&mutex_num_homens); //Down(mutex_num_homens)
     num_Homem--;
     homem_sai(id);
     if(num_Homem == 0)
     {
         sem_post(&banheiro); //Up(banheiro)
         puts("Banheiro livre");
     }
  sem_post(&mutex_num_homens); // Up(mutex_num_homens)
  free(idH);
  pthread_exit(NULL);
}

/*Funcao para alocar um novo identificador para uma pessoa*/
unsigned int *alocaId(unsigned int id)
{
  unsigned int *newId;  
  if ((newId = (unsigned int *)malloc(sizeof(unsigned int))) == NULL)
  {
     perror("Problemas na alocaÃ§Ã£o do identificador");
     exit(EXIT_FAILURE);
  }

  *newId = id;

  return newId;
}

int main(void)
{
  pthread_t idH[PESSOAS/2], idM[PESSOAS/2];

  register unsigned int i;

  /*Inicializa os semaforos*/
  sem_init(&mutex_num_mulheres, 0, 1);
  sem_init(&mutex_num_homens, 0, 1);
  sem_init(&banheiro, 0, 1);

  puts("Banheiro livre");

  /*Laco para criar os threads (um para cada pessoa)
   * metade dos threads simulam mulheres e outra metade
   * simula os homens
   */
  for (i=0; i<PESSOAS;i++)
  {
    if (i%2)
        pthread_create(&idH[i],NULL,Homem_quer_entrar,alocaId(i)); 
    else
        pthread_create(&idM[i],NULL,Mulher_quer_entrar,alocaId(i));
  }

  /* Laco para aguardar que todos os threads terminem*/
  for (i=0; i<PESSOAS;i++)
  {
    if (i%2)
        pthread_join(idH[i],NULL);
    else
        pthread_join(idM[i],NULL);
  }

  /*Desaloca os semaforos*/
  sem_destroy(&mutex_num_mulheres);
  sem_destroy(&mutex_num_homens);
  sem_destroy(&banheiro);

  pthread_exit(NULL);

  return 0;
}

