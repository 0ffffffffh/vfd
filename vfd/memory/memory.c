#include "..\include\memory.h"

#define VFD_POOL_TAG 'pdfV'

NTSTATUS MemAllocateNonPagedPool(
	__in ULONG Size,
	__out PVOID *Mem
	)
{

	if (Mem == NULL)
		return STATUS_UNSUCCESSFUL;

	*Mem = ExAllocatePoolWithTag(NonPagedPool,Size,VFD_POOL_TAG);

	if (*Mem == NULL)
		return STATUS_INSUFFICIENT_RESOURCES;

	return STATUS_SUCCESS;
}

NTSTATUS MemAllocatePagedPool(
	__in ULONG Size,
	__out PVOID *Mem
	)
{
	return MemAllocatePagedPoolEx(Size,Mem,FALSE,NULL);
}


NTSTATUS MemAllocatePagedPoolEx(
	__in ULONG Size,
	__out PVOID *Mem,
	__in BOOLEAN LockPages,
	__out PMDL *Mdl
	)
{
	PMDL mdl;
	PVOID vmem;

	if (Mem == NULL)
		return STATUS_UNSUCCESSFUL;

	vmem = ExAllocatePoolWithTag(PagedPool,Size,VFD_POOL_TAG);

	if (vmem == NULL)
		return STATUS_INSUFFICIENT_RESOURCES;

	if (LockPages)
	{
		mdl = IoAllocateMdl(vmem,Size,FALSE,FALSE,NULL);

		if (!mdl)
		{
			ExFreePoolWithTag(vmem,VFD_POOL_TAG);
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		 MmBuildMdlForNonPagedPool(mdl);

		__try
		{
			MmProbeAndLockPages(mdl,KernelMode,IoModifyAccess);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			IoFreeMdl(mdl);
			ExFreePoolWithTag(vmem,VFD_POOL_TAG);
			return GetExceptionCode();
		}
	}

	*Mem = vmem;

	return STATUS_SUCCESS;
}

VOID MemFreePoolMemory(
	__in PVOID Mem
	)
{
	ExFreePoolWithTag(Mem,VFD_POOL_TAG);
}


