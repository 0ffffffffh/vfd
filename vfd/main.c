#include ".\include\ntvfd.h"
#include ".\include\memory.h"
#include ".\include\mntmgr.h"


extern PDEVICE_OBJECT VfdDeviceObject = NULL;

UCHAR VdfServiceRoutines[3] =
{
	IRP_MJ_READ,
	IRP_MJ_WRITE,
	IRP_MJ_DEVICE_CONTROL
};

NTSTATUS DriverEntry(
	__in PDRIVER_OBJECT DriverObject,
	__in PIRP Irp
	);

VOID DriverUnload(
	__in PDRIVER_OBJECT DriverObject
	);

NTSTATUS VfdGenericIrpHandler(
	__in PDEVICE_OBJECT DevObj,
	__in PIRP Irp);

NTSTATUS VfdControllerDeviceIoCtl(
	__in PDEVICE_OBJECT DevObj,
	__in PIRP Irp
	);

NTSTATUS DriverEntry(
	__in PDRIVER_OBJECT DriverObject,
	__in PIRP Irp)
{
	NTSTATUS Status;
	UNICODE_STRING DeviceName;
	PDRIVER_DISPATCH *DriverIoTable;
	ULONG i;

	RtlInitUnicodeString(&DeviceName,VFD_DEVICE_NAME);

	Status = IoCreateDevice(DriverObject,0,&DeviceName,FILE_DEVICE_NULL,0,FALSE,&VfdDeviceObject);

	if (!NT_SUCCESS(Status))
	{
		NTFAILMSG(Status);
		return Status;
	}

	DriverIoTable = DriverObject->MajorFunction;

	for (i = 0; i<sizeof(VdfServiceRoutines); i++)
		*(DriverIoTable+VdfServiceRoutines[i]) = VfdGenericIrpHandler;

	DriverObject->DriverUnload = DriverUnload;

	Status = MntInitializeMountMgr(DriverObject);

	if (!NT_SUCCESS(Status))
	{
		NTFAILMSG(Status);
		IoDeleteDevice(VfdDeviceObject);
		VfdDeviceObject = NULL;
		return STATUS_UNSUCCESSFUL;
	}

	return STATUS_SUCCESS;
}

VOID DriverUnload(
	__in PDRIVER_OBJECT DriverObject
	)
{
	if (VfdDeviceObject != NULL)
		IoDeleteDevice(VfdDeviceObject);
}

NTSTATUS VfdGenericIrpHandler(
	__in PDEVICE_OBJECT DevObj,
	__in PIRP Irp)
{

	if (DevObj == VfdDeviceObject)
		return VfdControllerDeviceIoCtl(DevObj,Irp);
	
	return MntDispatchDiskIrp(DevObj->DeviceExtension,Irp);
}

VOID VfdCompleteIoRequestEx(
	__in PIRP Irp,
	__in NTSTATUS Status,
	__in ULONG_PTR Info
	)
{
	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = Info;

	IofCompleteRequest(Irp,IO_NO_INCREMENT);
}


VOID VfdCompleteIoRequest(
	__in PIRP Irp,
	__in NTSTATUS Status
	)
{
	VfdCompleteIoRequestEx(Irp,Status,0);
}

NTSTATUS VfdControllerDeviceIoCtl(
	__in PDEVICE_OBJECT DevObj,
	__in PIRP Irp
	)
{
	ULONG InputLength,OutputLength,Code;
	PVOID IoBuffer;
	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS Status = STATUS_SUCCESS;

	if (IrpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL)
	{
		Code = IrpStack->Parameters.DeviceIoControl.IoControlCode;
		InputLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
		OutputLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
		IoBuffer = Irp->AssociatedIrp.SystemBuffer;

		if (Irp->RequestorMode != KernelMode)
		{
			Status = MemValidateMemory(IoBuffer,InputLength,UserMode,PROBE_FOR_READ);

			if (!NT_SUCCESS(Status))
			{
				NTFAILMSGEX(Status,"Invalid memory 0x%p, size=%d",IoBuffer,InputLength);
				goto ret;
			}
		}

		/*
		switch (Code) 
		{
		default:

		}
		*/

	}
	else
		Status = STATUS_UNSUCCESSFUL;

ret:

	VfdCompleteIoRequest(Irp,Status);

	return Status;
}