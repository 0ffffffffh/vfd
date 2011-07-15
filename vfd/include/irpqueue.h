#ifndef __IRPQUEUE_H__
#define __IRPQUEUE_H__

#include "ntvfd.h"
#include "idw.h"


#define IqGetIrpQueueCount(IrpQueue) ((IrpQueue)->Count)

#define IqIsIrpQueueEmpty(IrpQueue) (((IrpQueue)->Count) == 0)

#define IqCancelWait(IrpQueue) KeSetEvent(&(IrpQueue)->CancelEvent,IO_NO_INCREMENT,FALSE)

typedef struct _IRPQUEUE_NODE
{
	struct _IRPQUEUE_NODE *Back;
	PIRP Irp;
}IRPQUEUE_NODE,*PIRPQUEUE_NODE;

typedef struct _IRPQUEUE
{
	PIRPQUEUE_NODE First;
	PIRPQUEUE_NODE Last;
	ULONG Count;
	KEVENT NotEmptyEvent;
	KEVENT CancelEvent;
	IRP_QUEUE_WORKER QueueWorker;
}IRPQUEUE,*PIRPQUEUE;


VOID IqInitializeIrpQueue(
	__in PIRPQUEUE Queue
	);

BOOLEAN IqInterlockedEnqueueIrp(
	__in PIRPQUEUE Queue,
	__in PIRP Irp
	);

BOOLEAN IqInterlockedDequeueIrp(
	__in PIRPQUEUE Queue,
	__out PIRP *Irp
	);

BOOLEAN IqEnqueueIrp(
	__in PIRPQUEUE Queue,
	__in PIRP Irp
	);

BOOLEAN IqDequeueIrp(
	__in PIRPQUEUE Queue,
	__out PIRP *Irp
	);


NTSTATUS IqWaitUntilQueueIsNotEmpty(
	__in PIRPQUEUE Queue
	);

#endif //__IRPQUEUE_H__