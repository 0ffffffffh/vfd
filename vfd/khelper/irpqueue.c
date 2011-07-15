#include "..\include\irpqueue.h"
#include "..\include\memory.h"

#define IqNotEmptyEvent 0
#define IqCancellationEvent 1

VOID IqInitializeIrpQueue(
	__in PIRPQUEUE Queue
	)
{
	ASSERT(Queue != NULL);

	KeInitializeEvent(&Queue->NotEmptyEvent,NotificationEvent,FALSE);
	KeInitializeEvent(&Queue->CancelEvent,NotificationEvent,FALSE);
	Queue->First = Queue->Last = NULL;
}

BOOLEAN IqInterlockedEnqueueIrp(
	__in PIRPQUEUE Queue,
	__in PIRP Irp
	)
{
	NOTIMPLEMENTED();
	return FALSE;
}

BOOLEAN IqInterlockedDequeueIrp(
	__in PIRPQUEUE Queue,
	__out PIRP *Irp
	)
{
	NOTIMPLEMENTED();
	return FALSE;
}

BOOLEAN IqEnqueueIrp(
	__in PIRPQUEUE Queue,
	__in PIRP Irp
	)
{
	NTSTATUS Status;
	BOOLEAN SetEvent=Queue->Count==0;
	PIRPQUEUE_NODE NewNode = NULL;

	Status = MemAllocateNonPagedPool(sizeof(IRPQUEUE_NODE),&NewNode);

	RtlZeroMemory(NewNode,sizeof(IRPQUEUE_NODE));

	if (!NT_SUCCESS(Status))
	{
		NTFAILMSG(Status);
		return FALSE;
	}

	NewNode->Irp = Irp;
	NewNode->Back = NULL;

	if (!Queue->Count)
	{
		Queue->First = Queue->Last = NewNode;
	}
	else
	{
		Queue->Last->Back = NewNode;
		Queue->Last = NewNode;
	}

	Queue->Count++;

	if (SetEvent)
		KeSetEvent(&Queue->NotEmptyEvent,IO_NO_INCREMENT,FALSE);

	return TRUE;
}

BOOLEAN IqDequeueIrp(
	__in PIRPQUEUE Queue,
	__out PIRP *Irp
	)
{
	PIRPQUEUE_NODE DeqNode;
	PIRP DeqIrp;

	if (Queue->Count == 0)
		return FALSE;

	DeqNode = Queue->First;
	Queue->First = DeqNode->Back;
	DeqIrp = DeqNode->Irp;

	if (DeqNode->Back == NULL)
		Queue->First = Queue->Last = NULL;

	Queue->Count--;

	MemFreePoolMemory(DeqNode);

	*Irp = DeqIrp;

	if (Queue->Count == 0)
		KeResetEvent(&Queue->NotEmptyEvent);

	return TRUE;
}

NTSTATUS IqWaitUntilQueueIsNotEmpty(
	__in PIRPQUEUE Queue
	)
{
	NTSTATUS Status;

	PVOID Events[2] =
	{
		&Queue->NotEmptyEvent,
		&Queue->CancelEvent
	};


	Status = KeWaitForMultipleObjects(2,Events,WaitAny,Executive,KernelMode,FALSE,NULL,0);

	switch (Status)
	{
	case IqNotEmptyEvent:
		KdbPrint(__FUNCTION__ ": An IRP enqueued");
		break;
	case IqCancellationEvent:
		KdbPrint(__FUNCTION__ ": Signalled cancellation event");
		break;
	}

	return Status;
}
