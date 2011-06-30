#include "..\include\vdisk.h"
#include "..\include\memory.h"

#define VdiInitializeVDiskItem(p,d,i,s) \
{ \
	(p)->VdevOb = d; \
	(p)->DeviceId = i; \
	(p)->VdevSize = s; \
} \


NTSTATUS VdiAllocateVDiskItem(
	__out PVDISK_OBJECT *VdiskObj
	)
{
	PVDISK_OBJECT disk;

	if (!NT_SUCCESS(MemAllocateNonPagedPool(sizeof(VDISK_OBJECT),&disk)))
		return STATUS_INSUFFICIENT_RESOURCES;

	*VdiskObj = disk;

	return STATUS_SUCCESS;
}

NTSTATUS VdiAllocateVirtualDisk(
	__in PDRIVER_OBJECT Controller,
	__in ULONG DeviceId,
	__in ULONG64 DeviceLength,
	__out PVDISK_OBJECT *DiskObj
	)
{
	NTSTATUS Status;
	UNICODE_STRING DevName;
	WCHAR DevNameBuff[256];
	PDEVICE_OBJECT DevObj;
	PVDISK_OBJECT NewDiskObj;

	RtlZeroMemory(DevNameBuff,0);

	RtlInitUnicodeString(&DevName,DevNameBuff);

	Status = IoCreateDevice(Controller,sizeof(DEVICE_EXTENSION_DATA),&DevName,FILE_DEVICE_DISK,0,FALSE,&DevObj);

	if (!NT_SUCCESS(Status))
	{
		NTFAILMSG(Status);
		return Status;
	}

	RtlZeroMemory(DevObj->DeviceExtension,sizeof(DEVICE_EXTENSION_DATA));

	((PDEVICE_EXTENSION_DATA)DevObj->DeviceExtension)->DeviceId = DeviceId;


	Status = VdiAllocateVDiskItem(&NewDiskObj);

	if (!NT_SUCCESS(Status))
	{
		NTFAILMSG(Status);
		return Status;
	}

	VdiInitializeVDiskItem(NewDiskObj,DevObj,DeviceId,DeviceLength);
	
	*DiskObj = NewDiskObj;

	DevObj->Flags |= DO_DIRECT_IO;
	DevObj->Flags &= ~DO_DEVICE_INITIALIZING;

	return STATUS_SUCCESS;
}