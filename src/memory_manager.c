/*
 * Universidade de Brasilia - DF
 * Departamento de Ciencia da Computacao - IE
 * Sistemas Operacionais - Turma A - 2/2010
 * Professora: Alba
 * Alunos:
 * 	- Bruno Pessanha de Carvalho - 08/25727
 * 	- Marcelo Valenca de Almeida - 08/35919
 *
 * Comentarios: Programa Memory Manager.
 *
 * Parametros: Os seguintes parametros sao passados por linha de comando caso a Flag Debug esteja 0:
 * 		OCUPACAO_OK
 * 		MAX_OCUPACAO
 * 		POLITICA
 * 		MAX_FRAMES
 * Onde politica pode ser:
 * 		RANDOM = 0,
 * 		FIFO = 1,
 * 		LRU = 2.
 *
 * Esse processo e responsavel pela criacao de estruturas de comunicacao com o Cliente,
 * Replacement Daemon e Shutdown Manager, alem da criacao do processo Replacement Daemon.
 * Ao iniciar, esse processo se duplica(fork()) e o processo filho sera o Replacement Daemon.
 * Dessa forma temos o PID do processo Daemon para uso futuro.
 * O processo pai criara uma memoria compartilhada com o tamanho de um inteiro para compartilhar
 * o numero de frames ocupados com o Replacement Daemon e outra memoria compartilhada onde 
 * as paginas do cliente serao armazenadas. Essa memoria e do tamanho dos Frames maximos vezes
 * o tamanho da estrutura que contem o PID do cliente e a pagina referenciada. Essa memoria e
 * compartilhada, pois o Replacement Daemon deve liberar itens seguindo uma determinada politica.
 *
 * Esse processo cria, ainda, uma memoria compartilhada com o Shutdown Manager
 * que contem uma lista de page faults de cada cliente(Limitamos o numero maximo de processos para 32,
 * mas isso pode ser alterado no arquivo shutdown_manager.h, alterando o define MAX_PROCCESS)
 * e uma outra memoria compartilhada de tamanho de 2 inteiros que contera o PID do deste processo
 * e o PID do Replacement Daemon. Dessa forma o Shutdown Manager pode imprimir um relatorio e
 * mandar sinais para este processo e para o Daemon. No caso o sinal que este processo recebe(SIGUSR1)
 * removera as filas,semaforos e memorias compartilhadas antes de ser encerrado.
 *
 * Em seguida e criado um semaforo que sera utilizado pelo Memory Manager e Replacement Daemon.
 * Esse semaforo ira sincronizar o uso da Lista compartilhada de Paginas e do numero de frames
 * ocupados, evitando assim dois processos na secao critica.
 *
 * Finalizando a criacao de estrutras de comunicacao, o Memory Manager cria uma Fila de Mensagens,
 * por onde o Cliente ira mandar mensagens contendo seu PID e o numero da pagina que deseja referenciar.
 *
 * O Memory Manager entao inica um loop "infinito" e fica bloqueado na primitiva msgrcv, esperando
 * que algum Cliente envie mensagens para ele.
 * Se nao houver espaco livre entao o Memory Manager fica encarregado de liberar espaco na lista
 * de paginas referenciadas e entao pode referenciar a nova pagina recebida pelo cliente.
 * Se houver espaco, entao a pagina e referenciada.
 *
 * Mecanismo de Comunicacao: Memoria Compartilhada e Filas de Mensagens.
 * Estrutura para Exclusao Mutua: Semaforo de Dijkstra
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
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>

#include "shutdown_manager.h"
#include "replacement_daemon.h"
#include "client.h"
#include "semaph.h"
#include "list.h"

#define DEBUG 0

list *frames;
listFaults *faults;
int32_t pageFaults;
int32_t idSem, idShM, idFaults, idPids, *pPids, idQueueClient, idFrames;

void clear(int32_t signal) {
	if (msgctl(idQueueClient, IPC_RMID, (struct msqid_ds *)NULL) < 0)
		printf("[ MEMORY MANAGER ] Erro ao remover fila de mensagens com o Cliente: %d.\n", errno);
	if (shmctl(idShM, IPC_RMID, (struct shmid_ds *)NULL) < 0)
		printf("[ MEMORY MANAGER ] Erro ao remover memoria compartilhada com o Daemon: %d.\n", errno);
	if (shmctl(idFaults, IPC_RMID, (struct shmid_ds *)NULL) < 0)
		printf("[ MEMORY MANAGER ] Erro ao remover memoria compartilhada com o Shutdown Manager 1: %d.\n", errno);
	if (shmctl(idPids, IPC_RMID, (struct shmid_ds *)NULL) < 0)
		printf("[ MEMORY MANAGER ] Erro ao remover memoria compartilhada com o Shutdown Manager 2: %d.\n", errno);
	if (shmctl(idFrames, IPC_RMID, (struct shmid_ds *)NULL) < 0)
		printf("[ MEMORY MANAGER ] Erro ao remover memoria compartilhada com o Daemon: %d.\n", errno);
	if (semctl(idSem, 0, IPC_RMID, 0) < 0)
		printf("[ MEMORY MANAGER ] Erro ao remover semaforo com o Daemon: %d.\n", errno);
	exit(EXIT_SUCCESS);
}

int32_t main(int32_t argc, char **argv) {
	message msgClient;
	int32_t processId;
	int32_t *pShM = NULL, i;
	char *OCUPACAO_OK;
	char *MAX_OCUPACAO;
	char *POLITICA;
	char *MAX_FRAMES;
	
	if (DEBUG == 1) {
		OCUPACAO_OK = "3";
		MAX_OCUPACAO = "6";
		POLITICA = "2";
		MAX_FRAMES = "12";
	}
	else {
		if(argc == 5) {
			OCUPACAO_OK = argv[1];
			MAX_OCUPACAO = argv[2];
			POLITICA = argv[3];
			MAX_FRAMES = argv[4];
		} else {
			printf("[ HELP ] ./mm 'OCUPACAO_OK' 'MAX_OCUPACAO' 'POLITICA' 'MAX_FRAMES'\n");
			exit(EXIT_FAILURE);
		}
	}

	system("clear");

	// Cria um processo filho.
	if((processId = fork()) < 0) {
		printf("[ MEMORY MANAGER ] Erro na criacao do processo Replacement Daemon: %d.\n", errno);
		exit(EXIT_FAILURE);
	}

	// Cria o processo Replacement Daemon.
	if(processId == 0) {
		if(execl("dm", "dm", OCUPACAO_OK, MAX_OCUPACAO, POLITICA, MAX_FRAMES, (char*)0) == -1) {
			printf("[ MEMORY MANAGER ] Erro na criacao do processo Replacement Daemon: %d.\n", errno);
			exit(EXIT_FAILURE);
		}		
	}

	// Cria memoria compartilhada com o Replacement Daemon para os framesOcupados
	if((idShM = shmget(KEY_DAEMON_FRAMES, sizeof(int32_t), IPC_CREAT|0x1FF)) == -1) {
		printf("[ MEMORY MANAGER ] Erro na criacao da memoria compartilhada com o Replacement Daemon: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	if((pShM = (int32_t *) shmat(idShM, (char*) 0, 0)) == (int32_t *) -1) {
		printf("[ MEMORY MANAGER ] Erro no mapeamento da memoria compartilhada com o Replacement Daemon: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	*pShM = 0;

	// Cria memoria compartilhada com o Replacement Daemon para a lista de framesOcupados
	if((idFrames = shmget(KEY_DAEMON_LIST, atoi(MAX_FRAMES)*sizeof(list), IPC_CREAT|0x1FF)) == -1) {
		printf("[ MEMORY MANAGER ] Erro na criacao da memoria compartilhada com o Replacement Daemon: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	if((frames = (list *) shmat(idFrames, (char*) 0, 0)) == (list *) -1) {
		printf("[ MEMORY MANAGER ] Erro no mapeamento da memoria compartilhada com o Replacement Daemon: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < atoi(MAX_FRAMES); i++) {
		frames[i].pid = -1; // FRAME LIVRE
	}

	// Cria memoria compartilhada com o Shutdown Manager para a lista de page faults
	if((idFaults = shmget(KEY_SHUTDOWN, MAX_PROCCESS*sizeof(listFaults), IPC_CREAT|0x1FF)) == -1) {
		printf("[ MEMORY MANAGER ] Erro na criacao da memoria compartilhada com o Shutdown Manager: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	if((faults = (listFaults *) shmat(idFaults, (char*) 0, 0)) == (listFaults *) -1) {
		printf("[ MEMORY MANAGER ] Erro no mapeamento da memoria compartilhada com o Shutdown Manager: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < MAX_PROCCESS; i++) {
		faults[i].pid = -1; // Lista de Page Fault de cada processo.
	}

	// Cria memoria compartilhada com o Shutdown Manager para o PID deste processo e o PID do Replacement Daemon
	if((idPids = shmget(KEY_SHUTDOWN+1, 2*sizeof(int32_t), IPC_CREAT|0x1FF)) == -1) {
		printf("[ MEMORY MANAGER ] Erro na criacao da memoria compartilhada com o Shutdown Manager: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	if((pPids = (int32_t *) shmat(idPids, (char*) 0, 0)) == (int32_t *) -1) {
		printf("[ MEMORY MANAGER ] Erro no mapeamento da memoria compartilhada com o Shutdown Manager: %d.\n", errno);
		exit(EXIT_FAILURE);
	}
	*pPids = getpid();
	pPids++;
	*pPids = processId;
	signal(SIGUSR1, clear);

	// Criacao do semaforo cmo o Replacement Daemon.
	if((idSem = semget(KEY_SEMAPHORE, 1, IPC_CREAT|0x1FF)) == -1) {
		printf("[ MEMORY MANAGER ] Erro na criacao do semaforo com o Replacement Daemon: %d.\n", errno);
		exit(EXIT_FAILURE);
	}

	// Cria a fila de mensagem com o cliente.
	if((idQueueClient = msgget(KEY_CLIENT, IPC_CREAT|0x1FF)) < 0) {
		printf("[ MEMORY MANAGER ] Erro na criacao da fila de mensagens com o cliente: %d.\n", errno);
		exit(EXIT_FAILURE);
	}

	pageFaults = 0;
	int32_t status;
	while(1) {
		status = msgrcv(idQueueClient, &msgClient, sizeof(message), 0, 0);
		if(status == -1) {
			printf("[ MEMORY MANAGER ] Erro no recebimento de mensagem do cliente %d: %d.\n", msgClient.pid, errno);
			exit(EXIT_FAILURE);
		}
		system("clear");
		printf("[ MEMORY MANAGER ] Mensagem recebida: %d do cliente %d.\n", msgClient.page, msgClient.pid);
		
		p_sem(idSem);
		if(*pShM == atoi(MAX_FRAMES)) {
			removePage(atoi(POLITICA), atoi(MAX_FRAMES), &frames);
			(*pShM)--;
			printf("[ MEMORY MANAGER ] Numero maximo de frames atingido.\n");
			printf("[ MEMORY MANAGER ] Pagina removida.\n");
		}
		v_sem(idSem);
		p_sem(idSem);
		if(!insertPage(msgClient, &frames, &faults, atoi(MAX_FRAMES), atoi(POLITICA))) {
			v_sem(idSem);
			printf("[ MEMORY MANAGER ] Impossivel inserir mensagem do cliente %d na fila: %d.\n", msgClient.pid, errno);
			printf("[ MEMORY MANAGER ] Pagina existente.\n");
		} else {
			(*pShM)++;
			v_sem(idSem);
			pageFaults++;
			printf("[ MEMORY MANAGER ] Page Faults = %d.\n", pageFaults);
		}
	}

	return EXIT_SUCCESS;
}
