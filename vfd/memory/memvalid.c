#include "..\include\memory.h"


#define MemValidateUserInputBuffer(Buff,Len,StatusVar,ValidatorPtr) \
{ \
	__try { \
		ValidatorPtr(Buff,Len,sizeof(UCHAR)); \
		*StatusVar = STATUS_SUCCESS; \
	} \
	__except(EXCEPTION_EXECUTE_HANDLER) { \
		*StatusVar = GetExceptionCode(); \
	} \
} \

BOOLEAN MemValidateForUserMode(
	__in PVOID Block,
	__in ULONG Size,
	__in UCHAR ProbeMode
	)
{
	NTSTATUS Status;
	BOOLEAN BothProbe = (ProbeMode & PROBE_FOR_BOTH) != FALSE;
	
	IRQLCHECK(KeGetCurrentIrql() > APC_LEVEL, DRIVER_IRQL_NOT_LESS_OR_EQUAL);

	if (ProbeMode & PROBE_FOR_READ)
	{
		MemValidateUserInputBuffer(Block,Size,&Status,ProbeForRead);

		if (!NT_SUCCESS(Status))
			return FALSE;

		if (!BothProbe)
			return TRUE;
	}
	

	if (ProbeMode & PROBE_FOR_WRITE)
	{
		MemValidateUserInputBuffer(Block,Size,&Status,ProbeForWrite);

		if (!NT_SUCCESS(Status))
			return FALSE;
	}

	return TRUE;
}

BOOLEAN MemValidateForKernelMode(
	__in PVOID Block,
	__in ULONG Size
	)
{
	NOTIMPLEMENTED();
	return FALSE;
}


NTSTATUS MemValidateMemory(
	__in PVOID MemoryBlock,
	__in ULONG Size,
	__in KPROCESSOR_MODE AccessMode,
	__in UCHAR ProbeMode
	)
{
	if (AccessMode != KernelMode)
		return MemValidateForUserMode(MemoryBlock,Size,ProbeMode) == TRUE ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;

	return MemValidateForKernelMode(MemoryBlock,Size);
}


NTSTATUS MemValidateMdlMemory(
	__in PMDL Mdl
	)
{
	return STATUS_NOT_IMPLEMENTED;
}