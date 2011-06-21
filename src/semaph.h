/*
 * Universidade de Brasilia - DF
 * Departamento de Ciencia da Computacao - IE
 * Sistemas Operacionais - Turma A - 2/2010
 * Professora: Alba
 * Alunos:
 * 	- Bruno Pessanha de Carvalho - 08/25727
 * 	- Marcelo Valenca de Almeida - 08/35919
 */

#ifndef _SEMAPH_
#define _SEMAPH_

#define KEY_SEMAPHORE 0x08359192

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

void p_sem(int32_t id);
void v_sem(int32_t id);

#endif
