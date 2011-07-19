//IRP DISPATCH WORKER SYSTEM THREAD

#include "..\include\ntvfd.h"
#include "..\include\irpqueue.h"
#include "..\include\idw.h"

VOID NTAPI IdwiWorkerRoutine(
	__in PVOID Context
	)
{
	PIRPQUEUE Queue;
	IRPQUEUE_COMPLETION_ROUTINE Completion;

	if (Context == NULL)
	{
		KdbPrint("Abnormal IDW termination. Context is null");
		PsTerminateSystemThread(STATUS_UNSUCCESSFUL);
		return;
	}

	Queue = (PIRPQUEUE)Context;
	Completion = Queue->QueueWorker.CompletionRoutine;

	while (InterlockedCompareExchange((volatile LONG *)&Queue->QueueWorker.WorkState,0,0) == 1)
	{
		IqWaitUntilQueueIsNotEmpty(Queue);
		Completion(Queue);
	}

	KdbPrint("Terminating IDW");

	PsTerminateSystemThread(STATUS_SUCCESS);
}


extern VFDINTERNAL NTSTATUS IdwCreateDispatcherThread(
	__in PIRPQUEUE Queue,
	__in IRPQUEUE_COMPLETION_ROUTINE CompletionCb
	)
{
	NTSTATUS Status;
	OBJECT_ATTRIBUTES Oat;
	ULONG Major=0,Minor=0;
	BOOLEAN XpAndLater=FALSE;

	NtGetOsVersion(&Major,&Minor);

	XpAndLater = ((Major >= 5 && Minor >= 1));

	if (XpAndLater) 
		InitializeObjectAttributes(&Oat, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

	InterlockedExchange((volatile LONG *)&Queue->QueueWorker.WorkState,1L);

	Queue->QueueWorker.CompletionRoutine = CompletionCb;

	Status = PsCreateSystemThread(&Queue->QueueWorker.Worker,GENERIC_ALL,XpAndLater ? &Oat : NULL,NULL,NULL,IdwiWorkerRoutine,Queue);

	if (!NT_SUCCESS(Status))
	{
		InterlockedExchange((volatile LONG *)&Queue->QueueWorker.WorkState,0L);
		NTFAILMSGEX(Status,"System dispatcher thread could not created");
		return Status;
	}

	return STATUS_SUCCESS;
}

extern VFDINTERNAL NTSTATUS IdwTerminateDispatcherThread(
	__in PIRPQUEUE Queue
	)
{
	InterlockedCompareExchange((volatile LONG *)&Queue->QueueWorker.WorkState,0,1);
	IqCancelWait(Queue);

	KeWaitForSingleObject(Queue->QueueWorker.Worker,Executive,KernelMode,FALSE,NULL);

	ZwClose(Queue->QueueWorker.Worker);

	return STATUS_SUCCESS;
}