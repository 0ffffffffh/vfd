#ifndef __VDISK_H__
#define __VDISK_H__

#include "ntvfd.h"
#include "irpqueue.h"

typedef struct __VDISK_OBJECT
{
	PDEVICE_OBJECT VdevOb;
	IRPQUEUE VdevIrpQueue;
	PLIST_ENTRY ContainedNode;
	ULONG DeviceId;
	ULONG64 VdevSize;
}VDISK_OBJECT,*PVDISK_OBJECT;

NTSTATUS VdiAllocateVirtualDisk(
	__in PDRIVER_OBJECT Controller,
	__in ULONG DeviceId,
	__in ULONG64 DeviceLength,
	__out PVDISK_OBJECT *DiskObj
	);

NTSTATUS VdiFreeVirtualDisk(
	__in PVDISK_OBJECT DiskObj
	);

NTSTATUS VdiDispatchDiskIrp(
	__in PVDISK_OBJECT DiskObj,
	__in PIRP Irp
	);

#endif //__VDISK_H__