/*
* Universidade de Brasilia - DF
* Departamento de Ciencia da Computacao - IE
* Sistemas Operacionais - Turma A - 2/2010
* Professora: Alba
* Alunos:
* 	- Bruno Pessanha de Carvalho - 08/25727
* 	- Marcelo Valenca de Almeida - 08/35919
*
* Comentario: Programa Semaph.
* Semaforo de Dijkstra.
*/

#include "semaph.h"

struct sembuf operacao[2];

void p_sem(int32_t id) {
	operacao[0].sem_num = 0;
	operacao[0].sem_op = 0;
	operacao[0].sem_flg = 0;
	operacao[1].sem_num = 0;
	operacao[1].sem_op = 1;
	operacao[1].sem_flg = 0;
	if (semop(id, operacao, 2) < 0)
		printf("[ SEMAPHORE ] erro no p=%d\n", errno);
}

void v_sem(int32_t id) {
	operacao[0].sem_num = 0;
	operacao[0].sem_op = -1;
	operacao[0].sem_flg = 0;
	if (semop(id, operacao, 1) < 0)
		printf("[ SEMAPHORE ] erro no v=%d\n", errno);
}
