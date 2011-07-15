#include "..\include\vdisk.h"
#include "..\include\vdskntrnl.h"
#include "..\include\memory.h"
#include "..\include\irpqueue.h"
#include <ntstrsafe.h>
#include <ntdddisk.h>


#define VdiInitializeVDiskItem(p,d,i,s) \
{ \
	(p)->VdevOb = d; \
	(p)->DeviceId = i; \
	(p)->VdevSize = s; \
} \

//FORWARDED ROUTINES BEGIN

VFDINTERNAL FORCEINLINE NTSTATUS VdisCheckBufferSize(
	__in ULONG IncomingSize,
	__in ULONG Required,
	__in PIO_STATUS_BLOCK IoStatus);

VFDINTERNAL NTSTATUS VdisGetIoParametersFromIrp(
	__in PIRP Irp,
	__out PIOPARAMETER Param
	);


//FORWARDED ROUTINES END

#define VdiGetIoCtlCodeFromIoParam(IoParam) ((ULONG)((IoParam)->Reserved.r0))
#define VdiGetIoCtlInputSizeFromIoParam(IoParam) ((ULONG)((IoParam)->Length))
#define VdiGetIoCtlOutputSizeFromIoParam(IoParam) ((ULONG)((IoParam)->Reserved.r1))


VFDINTERNAL NTSTATUS VdiAllocateVDiskItem(
	__out PVDISK_OBJECT *VdiskObj
	)
{
	PVDISK_OBJECT disk;

	if (!NT_SUCCESS(MemAllocateNonPagedPool(sizeof(VDISK_OBJECT),&disk)))
		return STATUS_INSUFFICIENT_RESOURCES;

	RtlZeroMemory(disk,sizeof(VDISK_OBJECT));

	*VdiskObj = disk;

	return STATUS_SUCCESS;
}

VFDINTERNAL NTSTATUS VdiFreeVDiskItem(
	__in PVDISK_OBJECT VdiskObj
	)
{
	if (VdiskObj == NULL)
		return STATUS_UNSUCCESSFUL;

	MemFreePoolMemory(VdiskObj);

	return STATUS_SUCCESS;
}

VFDINTERNAL NTSTATUS VdiHandleVdiskDeviceDevIo(
	__in PVDISK_OBJECT DiskObj,
	__in PIOPARAMETER IoParam,
	__out PIO_STATUS_BLOCK IrpStatus
	)
{
	ULONG CtlCode = VdiGetIoCtlCodeFromIoParam(IoParam);
	ULONG OutBuffSize = 0,InpBuffSize=0;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	ULONG Info = 0;

	switch (CtlCode)
	{
	case IOCTL_DISK_IS_WRITABLE:
		Status = STATUS_SUCCESS;
		break;
	case IOCTL_DISK_CHECK_VERIFY:
	case IOCTL_STORAGE_CHECK_VERIFY:
	case IOCTL_STORAGE_CHECK_VERIFY2:
		Status = STATUS_SUCCESS;
		break;
	case IOCTL_DISK_GET_DRIVE_GEOMETRY:
		{
			KdbPrint("Requested disk geometry for disk#%d",DiskObj->DeviceId);

			if (!NT_SUCCESS(Status = VdisCheckBufferSize(OutBuffSize,sizeof(DISK_GEOMETRY),IrpStatus)))
				break;
		}
		break;
	case IOCTL_DISK_GET_LENGTH_INFO:
		{
			PGET_LENGTH_INFORMATION Li;

			KdbPrint("Requested length info for disk#%d",DiskObj->DeviceId);

			if (!NT_SUCCESS(Status = VdisCheckBufferSize(OutBuffSize,sizeof(GET_LENGTH_INFORMATION),IrpStatus)))
				break;

			Li = (PGET_LENGTH_INFORMATION)IoParam->IoBuffer;

			Li->Length.QuadPart = DiskObj->VdevSize;

			Status = STATUS_SUCCESS;
			Info = sizeof(GET_LENGTH_INFORMATION);
		}
		break;
	case IOCTL_DISK_GET_DRIVE_LAYOUT:
		{
			PDRIVE_LAYOUT_INFORMATION Dli;

			KdbPrint("Requested disk partition info for disk%d",DiskObj->DeviceId);

			if (!NT_SUCCESS(Status = VdisCheckBufferSize(OutBuffSize,sizeof(DRIVE_LAYOUT_INFORMATION),IrpStatus)))
				break;

			Dli = (PDRIVE_LAYOUT_INFORMATION)IoParam->IoBuffer;

			Dli->PartitionCount = 1;
			Dli->Signature = 0xD1;
			
			Dli->PartitionEntry->BootIndicator = FALSE;
			Dli->PartitionEntry->HiddenSectors = 0; //FIX IT LATER
			Dli->PartitionEntry->PartitionLength.QuadPart = DiskObj->VdevSize;
			Dli->PartitionEntry->PartitionNumber = 1;
			Dli->PartitionEntry->PartitionType = PARTITION_ENTRY_UNUSED;
			Dli->PartitionEntry->RecognizedPartition = TRUE;
			Dli->PartitionEntry->RewritePartition = FALSE;
			Dli->PartitionEntry->StartingOffset.QuadPart = 0L;

			Info = sizeof(PARTITION_INFORMATION);
			Status = STATUS_SUCCESS;
		}
		break;
	default:
		KdbPrint("Unsupported disk ctl request (%x) for disk#%d",CtlCode,DiskObj->DeviceId);
	}

	IrpStatus->Status = Status;
	IrpStatus->Information = Info;

	return Status;
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
			Status = VdisGetIoParametersFromIrp(Irp,&IoParam);

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
		IoDeleteDevice(DevObj);
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
	IoDeleteDevice(DiskObj->VdevOb);
	return VdiFreeVDiskItem(DiskObj);
}


NTSTATUS VdiDispatchDiskIrp(
	__in PVDISK_OBJECT DiskObj,
	__in PIRP Irp
	)
{
	NTSTATUS Status;
	IOPARAMETER IoParam;

	Status = VdisGetIoParametersFromIrp(Irp,&IoParam);

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