#include "..\include\vdisk.h"
#include "..\include\memory.h"
#include "..\include\irpqueue.h"
#include <ntstrsafe.h>

typedef struct _IOPARAMETER
{
	UCHAR Major;
	ULONG Length;
	ULONG64 Offset;
	PUCHAR IoBuffer;
}IOPARAMETER,*PIOPARAMETER;

#define VdiInitializeVDiskItem(p,d,i,s) \
{ \
	(p)->VdevOb = d; \
	(p)->DeviceId = i; \
	(p)->VdevSize = s; \
} \


VFDINTERNAL NTSTATUS VdiAllocateVDiskItem(
	__out PVDISK_OBJECT *VdiskObj
	)
{
	PVDISK_OBJECT disk;

	if (!NT_SUCCESS(MemAllocateNonPagedPool(sizeof(VDISK_OBJECT),&disk)))
		return STATUS_INSUFFICIENT_RESOURCES;

	*VdiskObj = disk;

	return STATUS_SUCCESS;
}

VFDINTERNAL NTSTATUS VdiGetIoParametersFromIrp(
	__in PIRP Irp,
	__out PIOPARAMETER Param
	)
{
	PIO_STACK_LOCATION IoStack = NULL;
	struct 
	{
		ULONG Length;
		ULONG POINTER_ALIGNMENT Key;
		LARGE_INTEGER ByteOffset;
	}*IoOp = NULL;

	if (Param == NULL)
		return STATUS_UNSUCCESSFUL;

	RtlZeroMemory(Param,sizeof(IOPARAMETER));
	
	IoStack = IoGetCurrentIrpStackLocation(Irp);
	
	switch (IoStack->MajorFunction)
	{
	case IRP_MJ_WRITE:
		IoOp = &IoStack->Parameters.Write;
		break;
	case IRP_MJ_READ:
		IoOp = &IoStack->Parameters.Read;
		break;
	default:
		Param->IoBuffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;
		return STATUS_SUCCESS;
	}


	Param->Major = IoStack->MajorFunction;
	Param->IoBuffer = (PUCHAR)MmGetSystemAddressForMdlSafe(Irp->MdlAddress,NormalPagePriority);
	Param->Length = IoOp->Length;
	Param->Offset = IoOp->ByteOffset.QuadPart;

	return STATUS_SUCCESS;
}

VFDINTERNAL NTSTATUS VdiHandleVdiskDeviceDevIo(
	__in PVDISK_OBJECT DiskObj,
	__in PIOPARAMETER IoParam,
	__out PIO_STATUS_BLOCK IrpStatus
	)
{
	NOTIMPLEMENTED();
	return STATUS_NOT_IMPLEMENTED;
}

VFDINTERNAL NTSTATUS VdiDispatchVdiskDeviceRequest(
	__in PVDISK_OBJECT DiskObj,
	__in PIRP Irp
	)
{
	PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS Status;
	IOPARAMETER IoParam;

	switch (IoStack->MajorFunction)
	{
	case IRP_MJ_CREATE:
	case IRP_MJ_CLOSE:
		{
			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = 0;
		}
		break;
	case IRP_MJ_DEVICE_CONTROL:
		{
			Status = VdiGetIoParametersFromIrp(Irp,&IoParam);

			if (!NT_SUCCESS(Status))
				return Status;

			Status = VdiHandleVdiskDeviceDevIo(DiskObj,&IoParam,&Irp->IoStatus);
		}
		break;
	case IRP_MJ_QUERY_VOLUME_INFORMATION:
		{
			Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
			Irp->IoStatus.Information = 0;
		}
		break;
	}

	IofCompleteRequest(Irp,IO_NO_INCREMENT);

	return Status;
}

VFDINTERNAL VOID VdiCompleteAllPendingIrps(
	__in PVDISK_OBJECT Vdisk
	)
{
	PIRP PendingIrp;

	if (IqIsIrpQueueEmpty(&Vdisk->VdevIrpQueue))
		return;

	while (IqDequeueIrp(&Vdisk->VdevIrpQueue,&PendingIrp))
	{
		PendingIrp->IoStatus.Status = STATUS_SUCCESS;
		PendingIrp->IoStatus.Information = 0;

		IofCompleteRequest(PendingIrp,IO_NO_INCREMENT);
	}

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

	RtlStringCchPrintfW(DevNameBuff,sizeof(DevNameBuff),VFD_VDISK_DEV_FORMAT,DeviceId);

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
	
	IqInitializeIrpQueue(&NewDiskObj->VdevIrpQueue);

	*DiskObj = NewDiskObj;

	DevObj->Flags |= DO_DIRECT_IO;
	DevObj->Flags &= ~DO_DEVICE_INITIALIZING;

	return STATUS_SUCCESS;
}

NTSTATUS VdiFreeVirtualDisk(
	__in PVDISK_OBJECT DiskObj
	)
{
	VdiCompleteAllPendingIrps(DiskObj);
}


NTSTATUS VdiDispatchDiskIrp(
	__in PVDISK_OBJECT DiskObj,
	__in PIRP Irp
	)
{
	NTSTATUS Status;
	IOPARAMETER IoParam;

	Status = VdiGetIoParametersFromIrp(Irp,&IoParam);

	if (!NT_SUCCESS(Status))
		return Status;

	if (IoParam.Major)
	{
		IoMarkIrpPending(Irp);
		IqEnqueueIrp(&DiskObj->VdevIrpQueue,Irp);
		return STATUS_PENDING;
	}


	Status = VdiDispatchVdiskDeviceRequest(DiskObj,Irp);

	return Status;
}