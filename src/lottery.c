/*
*  lottery.c - Implementacao do algoritmo Lottery Scheduling e sua API
*
*  Autores: SUPER_PROGRAMADORES_C
*  Projeto: Trabalho Pratico I - Sistemas Operacionais
*  Organizacao: Universidade Federal de Juiz de Fora
*  Departamento: Dep. Ciencia da Computacao
*
*/

#include "lottery.h"
#include <stdio.h>
#include <string.h>

//Nome unico do algoritmo. Deve ter 4 caracteres.
const char lottName[]="LOTT";
int lottSchedSlot;
int totalTicketsReady = 0;


//=====Funcoes Auxiliares=====






//=====Funcoes da API=====

//Funcao chamada pela inicializacao do S.O. para a incializacao do escalonador
//conforme o algoritmo Lottery Scheduling
//Deve envolver a inicializacao de possiveis parametros gerais
//Deve envolver o registro do algoritmo junto ao escalonador
void lottInitSchedInfo() {
	SchedInfo *lottSchedInfo = malloc(sizeof(SchedInfo));
	strcpy(lottSchedInfo->name, lottName);
	lottSchedInfo->initParamsFn = lottInitSchedParams;
	lottSchedInfo->notifyProcStatusChangeFn = lottNotifyProcStatusChange;
    lottSchedInfo->scheduleFn = lottSchedule;
    lottSchedInfo->releaseParamsFn = lottReleaseParams;
	
	lottSchedSlot = schedRegisterScheduler(lottSchedInfo);
}

//Inicializa os parametros de escalonamento de um processo p, chamada 
//normalmente quando o processo e' associado ao slot de Lottery
void lottInitSchedParams(Process *p, void *params) {
	schedSetScheduler(p, params, lottSchedSlot);
}

//Recebe a notificação de que um processo sob gerência de Lottery mudou de estado
//Deve realizar qualquer atualização de dados da Loteria necessária quando um processo muda de estado
void lottNotifyProcStatusChange(Process* p) {
	LotterySchedParams *params = processGetSchedParams(p);
	if (processGetStatus(p) == PROC_READY) 
		totalTicketsReady += params->num_tickets;		
	else if (processGetStatus(p) != PROC_WAITING) // assume que somente um proceso nao vai pra waiting se estiver ready
		totalTicketsReady -= params->num_tickets;
}

//Retorna o proximo processo a obter a CPU, conforme o algortimo Lottery 
Process* lottSchedule(Process *plist) {
	if (totalTicketsReady == 0) return NULL;
	int winner = rand() % (totalTicketsReady) + 1;
	int offset = 0;

	for (Process* p = plist; p != NULL; p = processGetNext(p)) {
		if (processGetStatus(p) == PROC_READY) {
			LotterySchedParams *params = processGetSchedParams(p);
			offset += params->num_tickets;
			if (winner <= offset) 
				return p;
		}
	}
	return NULL;
}

//Libera os parametros de escalonamento de um processo p, chamada 
//normalmente quando o processo e' desassociado do slot de Lottery
//Retorna o numero do slot ao qual o processo estava associado
int lottReleaseParams(Process *p) {
	if (processGetStatus(p) == PROC_READY) {
		LotterySchedParams *params = processGetSchedParams(p);
		totalTicketsReady -= params->num_tickets;
	}
	processSetSchedSlot(p, -1);
	processSetSchedParams(p, NULL);
	free(processGetSchedParams(p));
	return lottSchedSlot;
}

//Transfere certo numero de tickets do processo src para o processo dst.
//Retorna o numero de tickets efetivamente transfeirdos (pode ser menos)
int lottTransferTickets(Process *src, Process *dst, int tickets) {
	LotterySchedParams *srcParams = processGetSchedParams(src);
	LotterySchedParams *dstParams = processGetSchedParams(dst);
	int transferred = (srcParams->num_tickets >= tickets) ? tickets : srcParams->num_tickets;

	srcParams->num_tickets -= transferred;
	dstParams->num_tickets += transferred;

	if (processGetStatus(src) == PROC_READY && processGetStatus(dst) != PROC_READY)
		totalTicketsReady -= transferred;
	else if (processGetStatus(dst) == PROC_READY && processGetStatus(src) != PROC_READY)
		totalTicketsReady += transferred;
	
	return transferred;
}
