/*
 * Universidade de Brasilia - DF
 * Departamento de Ciencia da Computacao - IE
 * Sistemas Operacionais - Turma A - 2/2010
 * Professora: Alba
 * Alunos:
 * 	- Bruno Pessanha de Carvalho - 08/25727
 * 	- Marcelo Valenca de Almeida - 08/35919
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "list.h"
#include "shutdown_manager.h"

/*Funcao que libera um frame seguindo uma determinada politica*/
void removePage(int32_t politica, int32_t max_frames, list **frames) {
	time_t aux;
	int32_t index = 0, i;
	switch(politica) {
		case FIFO:
		case LRU:
		{
			aux = time(NULL);

			/* Procura Frame Ocupado que possui o menor time stamp.
			Se o time stamp for 0, aquele frame ainda não foi utilizado */
			for(i = 0; i < max_frames; i++){
				if(((*frames)[i].timeStamp > 0) && ((*frames)[i].pid != -1) && ((*frames)[i].timeStamp <= aux)){
					aux = (*frames)[i].timeStamp;
					index = i;
				}
			}
			break;
		}
		case RANDOM:
		{
			srandom(time((time_t *)NULL));
			index = random() % max_frames;
			while((*frames)[index].pid == -1) //Procura Frame Ocupado.
				index = random() % max_frames;
			break;
		}
	}
	(*frames)[index].pid = -1; //Frame Livre
	(*frames)[index].page = -1;
	(*frames)[index].timeStamp = 0;
}

void faultsPlusPlus(listFaults *faults, int32_t pid) {
	int i;
	
	for(i = 0; i < MAX_PROCCESS; i++) {
		if(faults[i].pid == pid) {
			faults[i].faults++;
			return;
		}
		if(faults[i].pid == -1) { //Caso seja um novo processo
			faults[i].faults = 1;
			faults[i].pid = pid;
			return;
		}
	}
}

/*Aloca pagina em um frame disponível. O frame disponivel é escolhido aleatoriamente.*/
int32_t insertPage(message msg, list **frames, listFaults **faults, int32_t max_frames, int32_t politica) {
	list *aux = *frames;
	int i, livre = 0, livres = 0, index;

	for(i = 0; i < max_frames; i++) {
		if((aux[i].page == msg.page) && (aux[i].pid == msg.pid)) {
			if(politica == LRU)
				aux[i].timeStamp = time(NULL); // Caso a politica seja LRU, então atualiza o timeStamp daquela pagina
			return 0; /*Página já alocada*/
		}
		if(aux[i].pid == -1)
			livres++;
	}
	i = 0;
	srandom(time((time_t *)NULL));
	index = random() % livres;
	do{
		if((*frames)[i].pid == -1) {
			if(livre == index) {
				(*frames)[i].page = msg.page;
				(*frames)[i].pid = msg.pid;
				(*frames)[i].timeStamp = time(NULL); // Seta time stamp
				faultsPlusPlus(*faults, msg.pid); // Incrementa os Page faults
				return 1;
			}
			livre++;
		}
		i++;
	} while(i < max_frames);

	return -1; 
}