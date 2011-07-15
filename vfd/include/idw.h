#ifndef __IDW_H__
#define __IDW_H__


struct PIRPQUEUE;

typedef VOID (*IRPQUEUE_COMPLETION_ROUTINE)(PIRPQUEUE);

typedef struct _IRP_QUEUE_WORKER
{
	HANDLE							Worker;
	IRPQUEUE_COMPLETION_ROUTINE		CompletionRoutine;
	volatile LONG					WorkState;
}IRP_QUEUE_WORKER,*PIRP_QUEUE_WORKER;

#endif //__IDW_H__