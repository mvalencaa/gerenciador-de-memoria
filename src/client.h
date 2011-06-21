/*
 * Universidade de Brasilia - DF
 * Departamento de Ciencia da Computacao - IE
 * Sistemas Operacionais - Turma A - 2/2010
 * Professora: Alba
 * Alunos:
 * 	- Bruno Pessanha de Carvalho - 08/25727
 * 	- Marcelo Valenca de Almeida - 08/35919
 */

#ifndef _CLIENT_
#define _CLIENT_

#define KEY_CLIENT 0x0835919

typedef struct message {
	long msgType;
	int pid;
	int page;
} message;

#endif
