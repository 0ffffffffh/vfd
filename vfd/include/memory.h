#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "..\include\ntvfd.h"

#define PROBE_DO_NOT 0x0
#define PROBE_FOR_READ 0x2
#define PROBE_FOR_WRITE 0x4
#define PROBE_FOR_BOTH (PROBE_FOR_READ | PROBE_FOR_WRITE)

NTSTATUS MemAllocateNonPagedPool(
	__in ULONG Size,
	__out PVOID *Mem
	);

NTSTATUS MemAllocatePagedPool(
	__in ULONG Size,
	__out PVOID *Mem
	);

NTSTATUS MemAllocatePagedPoolEx(
	__in ULONG Size,
	__out PVOID *Mem,
	__in BOOLEAN LockPages,
	__out PMDL *Mdl
	);

VOID MemFreePoolMemory(
	__in PVOID Mem
	);


//MEMORY VALIDATORS

NTSTATUS MemValidateMemory(
	__in PVOID MemoryBlock,
	__in ULONG Size,
	__in KPROCESSOR_MODE AccessMode,
	__in UCHAR ProbeMode
	);

#endif //__MEMORY_H__