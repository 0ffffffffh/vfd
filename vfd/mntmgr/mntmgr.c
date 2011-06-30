#include "..\include\mntmgr.h"
#include "..\include\vdisk.h"

KMUTEX MntGeneralMountLock;
PDRIVER_OBJECT MntDriverObject;

typedef struct _VDISK_LIST_ENTRY
{
	SINGLE_LIST_ENTRY Entry;
	PVDISK_OBJECT VDisk;
}VDISK_LIST_ENTRY,*PVDISK_LIST_ENTRY;

#define MntAcquireGeneralLock() KeWaitForSingleObject(&MntGeneralMountLock,Executive,KernelMode,FALSE,NULL)

#define MntReleaseGeneralLock() KeReleaseMutex(&MntGeneralMountLock,FALSE)

VOID MntRegisterDiskDeviceObject(
	__in PVDISK_OBJECT Vdisk
	)
{
	NOTIMPLEMENTED();
}

NTSTATUS MntInitializeMountMgr(
	__in PDRIVER_OBJECT DriverObject
	)
{
	NTSTATUS Status;
	UNICODE_STRING DevDirName;
	OBJECT_ATTRIBUTES ObjAttrib;
	HANDLE DevObjHandle;

	KeInitializeMutex(&MntGeneralMountLock,0);
	
	RtlInitUnicodeString(&DevDirName,VFD_ROOT_DIR_DEV);

	InitializeObjectAttributes(&ObjAttrib,&DevDirName,OBJ_OPENIF,NULL,NULL);

	Status = ZwCreateDirectoryObject(&DevObjHandle, DIRECTORY_ALL_ACCESS, &ObjAttrib);

	if (!NT_SUCCESS(Status))
		return Status;

	MntDriverObject = DriverObject;

	return STATUS_SUCCESS;
}

NTSTATUS MntMountDisk(
	__in ULONG64 DiskLength,
	__out PULONG VdiskId
	)
{
	NTSTATUS Status;

	PVDISK_OBJECT DiskObj;

	MntAcquireGeneralLock();

	Status = VdiAllocateVirtualDisk(MntDriverObject,0,DiskLength,&DiskObj);

	if (!NT_SUCCESS(Status))
	{
		Status = STATUS_UNSUCCESSFUL;
		goto Exit;
	}

	MntRegisterDiskDeviceObject(DiskObj);

	*VdiskId = DiskObj->DeviceId;

Exit:

	MntReleaseGeneralLock();

	return Status;
}

NTSTATUS MntUnmountDisk(
	__in ULONG VdiskId
	)
{
	MntAcquireGeneralLock();

	NOTIMPLEMENTED();

	MntReleaseGeneralLock();

	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS MntDispatchDiskIrp(
	__in PVOID DeviceInfo,
	__in PIRP Irp
	)
{
	MntAcquireGeneralLock();

	NOTIMPLEMENTED();

	MntReleaseGeneralLock();

	return STATUS_NOT_IMPLEMENTED;
}