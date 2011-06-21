/*
* Universidade de Brasilia - DF
* Departamento de Ciencia da Computacao - IE
* Sistemas Operacionais - Turma A - 2/2010
* Professora: Alba
* Alunos:
* 	- Bruno Pessanha de Carvalho - 08/25727
* 	- Marcelo Valenca de Almeida - 08/35919
*
* Comentario: Programa Shutdown Manager.
* Este programa e responsavel por encerrar a execucao dos processos
* memory_manager e replacement_daemon, bem como remover as estruturas dos
* mecanismos de comunicacao utilizados durante a execucao dos processos. Para
* tal, sao utilizadas duas areas de memoria compartilhada, cujas
* funcionalidades sao:
*
* - Passar o ponteiro para o inicio da lista de page faults (variavel "faults");
* - Passar o valor dos pids dos processos a serem encerrados (variavel "pPids").
*
* De posse desses valores, sao entao lancados sinais para cada um dos processos:
*
* - SIGUSR1: Sinal lancado ao processo Memory Manager. Sua rotina de tratamento
*            remove a fila, as memorias compartilhadas e os semaforos;
* - SIGTERM: Sinal lancado ao processo Replacement Daemon. Tem como funcao
*            encerrar o processo.
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
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>

#include "list.h"
#include "shutdown_manager.h"

void printPageFaults(listFaults *faults) {
	int32_t total = 0;
	int i;
	
	system("clear");
	printf("[ SHUTDOWN MANAGER ]\n");
	printf("\nProcesso\t\tFaults\n");
	for(i = 0; i < MAX_PROCCESS; i++) {
		if(faults[i].pid != -1) {
			printf("  %d\t\t\t  %d\n", faults[i].pid, faults[i].faults);
			total += faults[i].faults;
		}
	}
	printf("\nTotal de page faults: %d\n", total);
}

int main() {
	int idFaults, idPids, *pPids;
	listFaults *faults;
	
	// Acessa memoria compartilhada com o Memory Manager para a lista de page faults
	if((idFaults = shmget(KEY_SHUTDOWN, MAX_PROCCESS*sizeof(listFaults), 0x1FF)) == -1) {
		printf("[ SHUTDOWN MANAGER ] Erro no acesso da memoria compartilhada com o Memory Manager: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	if((faults = (listFaults *) shmat(idFaults, (char*) 0, 0)) == (listFaults *) -1) {
		printf("[ SHUTDOWN MANAGER ] Erro no mapeamento da memoria compartilhada com o Memory Manager: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	
	// Cria memoria compartilhada com o Memory Manager para a lista de page faults
	if((idPids = shmget(KEY_SHUTDOWN+1, 2*sizeof(int32_t), 0x1FF)) == -1) {
		printf("[ SHUTDOWN MANAGER ] Erro no acesso da memoria compartilhada com o Memory Manager: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	if((pPids = (int32_t *) shmat(idPids, (char*) 0, 0)) == (int32_t *) -1) {
		printf("[ SHUTDOWN MANAGER ] Erro no mapeamento da memoria compartilhada com o Memory Manager: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	
	kill(*pPids, SIGUSR1); //Envia sinal ao Memory Manager para a remocao de fila, memoria compartilhada e semaforos.
	pPids++;
	kill(*pPids, SIGTERM); //Encerra o daemon
	
	printPageFaults(faults);
	return EXIT_SUCCESS;
}
