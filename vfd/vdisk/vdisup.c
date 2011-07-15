#include "..\include\vdisk.h"
#include "..\include\vdskntrnl.h"

//SUPPORT ROUTINES

extern VFDINTERNAL FORCEINLINE NTSTATUS VdisCheckBufferSize(
	__in ULONG IncomingSize,
	__in ULONG Required,
	__in PIO_STATUS_BLOCK IoStatus)
{
	if (IncomingSize < Required)
	{
		IoStatus->Status = STATUS_BUFFER_TOO_SMALL;
		IoStatus->Information = Required;
		return STATUS_BUFFER_TOO_SMALL;
	}

	return STATUS_SUCCESS;
}

extern VFDINTERNAL NTSTATUS VdisGetIoParametersFromIrp(
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
	case IRP_MJ_DEVICE_CONTROL:
		{
			Param->IoBuffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;
			Param->Reserved.r0 = (ULONG_PTR)IoStack->Parameters.DeviceIoControl.IoControlCode;
			Param->Length = IoStack->Parameters.DeviceIoControl.InputBufferLength;
			Param->Reserved.r1 = (ULONG_PTR)IoStack->Parameters.DeviceIoControl.OutputBufferLength;
			return STATUS_SUCCESS;
		}
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