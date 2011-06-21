/*
 * Universidade de Brasilia - DF
 * Departamento de Ciencia da Computacao - IE
 * Sistemas Operacionais - Turma A - 2/2010
 * Professora: Alba
 * Alunos:
 * 	- Bruno Pessanha de Carvalho - 08/25727
 * 	- Marcelo Valenca de Almeida - 08/35919
 *
 * Comentarios: Programa Cliente.
 * Esse processo é responsável por pedir ao Memory Manager para referenciar uma pagina.
 * Para isso, o processo utiliza a fila de mensagens criada pelo Memory Manager.
 * A mensagem enviada para o Memory Manager contém o PID deste processo e o numero
 * da página. As paginas estarao em um arquivo com nome: paginas_processo_n e esse
 * arquivo e passado para este programa como parametro via linha de comando.
 * 
 * Mecanismo de Comunicação: Fila de Mensagens.
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
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#include "client.h"

message makeMsg(int32_t page);
void leave();

int32_t main(int32_t argc, char **argv) {

	FILE *fp;

	int32_t idQueueMemoryManager;
	int32_t page;
	message msg;
	char c;

	if(argc == 2) {
		fp = fopen(argv[1], "r");
		if(fp != NULL) {
			do {
				fscanf(fp, "%d", &page);
				c = getc(fp);

				if((idQueueMemoryManager = (msgget(KEY_CLIENT, 0x1FF))) < 0) {
					printf("[ CLIENT ] Erro na criacao da fila de mensagens com o Memory Manager: %d.\n", errno);
					exit(EXIT_FAILURE);
				}

				msg = makeMsg(page);
				
				if(msgsnd(idQueueMemoryManager, &msg, sizeof(message), 0) < 0) {
					printf("[ CLIENT ] Erro ao enviar pagina %d para o Memory Manager: %d.\n", page, errno);
				}
				sleep(5);
			} while(c != EOF);
		} else
			printf("[ CLIENT ] Arquivo não encontrado.\n");
			exit(EXIT_FAILURE);
	} else {
		printf("[ CLIENT ] Erro ao iniciar Cliente.\n");
		printf("[ HELP ] ./cliente arquivo\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

message makeMsg(int32_t page) {
	message msg;

	msg.pid = getpid(); //TODO Refatorar! Dessa forma ele sempre chama o procedimento!
	msg.page = page;

	return msg;
}

