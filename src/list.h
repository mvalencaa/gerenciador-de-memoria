/*
 * Universidade de Brasilia - DF
 * Departamento de Ciencia da Computacao - IE
 * Sistemas Operacionais - Turma A - 2/2010
 * Professora: Alba
 * Alunos:
 * 	- Bruno Pessanha de Carvalho - 08/25727
 * 	- Marcelo Valenca de Almeida - 08/35919
 */

#ifndef _LIST_
#define _LIST_

#include <inttypes.h>
#include <time.h>

#include "client.h"

typedef struct list {
	int32_t page;
	int32_t pid;
	time_t timeStamp;
}list;

typedef struct listF {
	int32_t faults;
	int32_t pid;
}listFaults;

enum politicEnum {
	RANDOM = 0,
	FIFO = 1,
	LRU = 2
};

void removePage(int32_t politica, int32_t max_frames, list **framesList);
int32_t insertPage(message msg, list **frames, listFaults **faults, int32_t max_frames, int32_t politica);

#endif