/*
* Universidade de Brasilia - DF
* Departamento de Ciencia da Computacao - IE
* Sistemas Operacionais - Turma A - 2/2010
* Professora: Alba
* Alunos:
* 	- Bruno Pessanha de Carvalho - 08/25727
* 	- Marcelo Valenca de Almeida - 08/35919
*
* Comentario: Programa Replacement Daemon.
* Este programa e responsavel por verificar de tempos em tempos como esta a
* ocupacao de paginas nos frames criados. No momento em que essa ocupacao
* estiver maior ou igual ao parametro MAX_OCUPACAO, este processo deve iniciar
* um procedimento de remocao de paginas ate que a quantidade de frames ocupados
* seja menor que o parametro OCUPACAO_OK. Para tal, sao utilizadas duas areas
* de memoria compartilhada, cujas funcionalidades sao:
*
* - Passar o valor da quantidade de frames ocupados (variavel "pShM");
* - Passar o ponteiro para o inicio do vetor de frames (variavel "frames").
*
* O processo de remocao pode ter seu comportamento alterado, dependendo do
* valor do parametro POLITICA:
*
* - FIFO:		Remove a primeira pagina alocada;
* - LRU;		Remove a pagina "menos recentemente utilizada";
* - RANDOM;	Remove uma pagina aleatoria.
*
* Para um correto funcionamento do processo, fez-se necessaria a utilizacao de
* um semaforo para controle das secoes criticas.
*
* Sistema Operacional:
* 	Ubuntu 10.04(lucid) 64 bits
*	Kernel Linux 2.6.32-26-generic
*	GNOME 2.30.2
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>

#include "replacement_daemon.h"
#include "semaph.h"
#include "list.h"

int32_t main(int32_t argc, char **argv) {
	
	int32_t OCUPACAO_OK = atoi(argv[1]);
	int32_t MAX_OCUPACAO = atoi(argv[2]);
	int32_t POLITICA = atoi(argv[3]);
	int32_t MAX_FRAMES = atoi(argv[4]);
	int32_t idShM;
	int32_t idFrames;
	int32_t idSem;
	list *frames;
	int32_t *pShM = NULL;
	
	// Recupera area de memoria compartilhada com o Memory Manager.
	if((idShM = shmget(KEY_DAEMON_FRAMES, sizeof(int32_t), 0x1FF)) < 0) {
		printf("[ REPLACEMENT DAEMON ] Erro na recuperacao da memoria compartilhada com o Memory Manager: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	if((pShM = (int32_t*) shmat(idShM, (char*) 0, 0)) == (int32_t*)-1) {
		printf("[ REPLACEMENT DAEMON ] Erro no mapeamento da memoria compartilhada com o Memory Manager: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	
	// Recupera area memoria compartilhada com o Memory Manager para a lista de frames
	if((idFrames = shmget(KEY_DAEMON_LIST, MAX_FRAMES*sizeof(list), 0x1FF)) == -1) {
		printf("[ REPLACEMENT DAEMON ] Erro na recuperacao da memoria compartilhada com o Memory Manager: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	if((frames = (list *) shmat(idFrames, (char*) 0, 0)) == (list *) -1) {
		printf("[ REPLACEMENT DAEMON ] Erro no mapeamento da memoria compartilhada com o Memory Manager: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	
	// Criacao do semaforo com o Memory Manager.
	if((idSem = semget(KEY_SEMAPHORE, 1, 0x1FF)) == -1) {
		printf("[ REPLACEMENT DAEMON ] Erro na criacao do semaforo com o Memory Manager: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	
	printf("[ REPLACEMENT DAEMON ] Executando Replacement Daemon...\n");
	do {
		p_sem(idSem);
		if(*pShM >= MAX_OCUPACAO) {
			while(*pShM >= OCUPACAO_OK) {
				removePage(POLITICA, MAX_FRAMES, &frames);
				(*pShM)--;
				printf("[ REPLACEMENT DAEMON ] Remove página. Frames Ocupados: %d.\n", *pShM);
				v_sem(idSem);
				//sleep(1);
				p_sem(idSem);
			}
		}
		v_sem(idSem);
		sleep(2);
	} while(1);
	
	return EXIT_SUCCESS;
}
