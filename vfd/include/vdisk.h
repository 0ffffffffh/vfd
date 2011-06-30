#ifndef __VDISK_H__
#define __VDISK_H__

#include "ntvfd.h"

typedef struct __VDISK_OBJECT
{
	PDEVICE_OBJECT VdevOb;
	ULONG DeviceId;
	ULONG64 VdevSize;
}VDISK_OBJECT,*PVDISK_OBJECT;


NTSTATUS VdiAllocateVirtualDisk(
	__in PDRIVER_OBJECT Controller,
	__in ULONG DeviceId,
	__in ULONG64 DeviceLength,
	__out PVDISK_OBJECT *DiskObj
	);

#endif //__VDISK_H__