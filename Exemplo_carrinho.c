/*Solucao da 11a questão da lista de exercicios, 
 * que simula um carrinho de montanha russa.
 */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define VEZES 5 // Define a quantidade de vezes que o carrinho vai passear
#define PASSAGEIROS 5 // Define a quantidade de passageiros
#define PASSAGENS 5 // Define a quantidade de voltas do carrinho
#define CAPACIDADE 5 // Define a capacidade do carrinho

/*Declaracao dos semaforos necessarios*/
sem_t semaf_passageiros; // Controla os passageiros
sem_t semaf_libera; // Libera o carrinho para dar uma volta
sem_t semaf_quant;  // Controla o acesso a quant_passageiros
sem_t semaf_espera; // Bloqueia a entrada de passageiro

/*Variavel para contar a quantidade de passageiros*/
unsigned int quant_passageiros=0;

/*Funcao que libera o carrinho para mais uma volta*/
void libera_carrinho();

/* Thread para simular o comportamento do passageiro*/
void *passageiro(void* arg)
{
  register unsigned int i = 0;

  while (i++ < PASSAGENS)
  {
    sem_wait(&semaf_passageiros); // Down(semaf_passageiros)
    puts("Passageiro entrou no carrinho!");
    sem_wait(&semaf_quant); // Down(semaf_quant)
       quant_passageiros++;
       printf("Quantidade de passageiros: %d\n", quant_passageiros);
       if (quant_passageiros == CAPACIDADE)
       {
         puts("Liberando carrinho");
         sem_post(&semaf_libera); // Up(semaf_libera)
         sem_post(&semaf_quant);  // Up(semaf_quant)
         sem_wait(&semaf_espera); // Down(semaf_espera)
       }
       else
         sem_post(&semaf_quant); // Up(semaf_quant)

    sem_post(&semaf_passageiros);// Up(semaf_passageiros)
  }

  pthread_exit(NULL);
}

/*Thread para simular o comportamento do carrinho*/
void *carrinho(void *arg)
{
  register unsigned int i = 0;

  while (i++<VEZES)
  {
    puts("Carrinho aguardando");  
    sem_wait(&semaf_libera); // Down(semaf_libera)
       libera_carrinho();
       sem_wait(&semaf_quant); // Down(semaf_quant)
          quant_passageiros = 0;
       sem_post(&semaf_quant); // Up(semaf_quant)
    sem_post(&semaf_espera); // Up(semaf_espera)
  }

  pthread_exit(NULL);
}

/*Funcao que libera o carrinho*/
void libera_carrinho()
{
  puts("Carrinho liberado");

  // Carrinho dando a volta
  puts("---->");
  sleep(1);
  puts("Carrinho voltando");
  puts("<----");
}

int main(void)
{
  pthread_t passageiros[PASSAGEIROS];
  pthread_t idCarrinho;

  register unsigned int i;

  /*Inicializa os semaforos*/
  sem_init(&semaf_passageiros, 0, 1);
  sem_init(&semaf_libera, 0, 0);
  sem_init(&semaf_quant, 0, 1);
  sem_init(&semaf_espera, 0, 0);

  /*Cria o thread do carrinho*/
  pthread_create(&idCarrinho, NULL, carrinho, NULL);

  /*Cria varios threads, um para cada passageiro*/
  for (i=0; i<PASSAGEIROS;i++)
        pthread_create(&passageiros[i], NULL, passageiro, NULL); 

  /*Aguarda os threads dos passageiros terminarem*/
  for (i=0; i<PASSAGEIROS;i++)
        pthread_join(passageiros[i],NULL);

  /*Aguarda o thread do carrinho terminar*/
  pthread_join(idCarrinho, NULL);

  /*Desaloca os semaforos*/
  sem_destroy(&semaf_passageiros);
  sem_destroy(&semaf_libera);
  sem_destroy(&semaf_quant);
  sem_destroy(&semaf_espera);

  pthread_exit(NULL);

  return 0;
}

