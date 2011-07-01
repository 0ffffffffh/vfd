#include "..\include\mntmgr.h"
#include "..\include\vdisk.h"
#include "..\include\memory.h"

KMUTEX MntGeneralMountLock;
PDRIVER_OBJECT MntDriverObject;
ULONG MntDeviceCount=0;
SINGLE_LIST_ENTRY MntDiskList;

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
	NTSTATUS Status;
	PVDISK_LIST_ENTRY ListEntry = NULL;
	
	Status = MemAllocateNonPagedPool(sizeof(VDISK_LIST_ENTRY),&ListEntry);

	ListEntry->VDisk = Vdisk;

	if (!NT_SUCCESS(Status))
	{
		NTFAILMSG(Status);
		return;
	}

	PushEntryList(&MntDiskList,&ListEntry->Entry);
}

BOOLEAN MntDeRegisterDiskDeviceObject(
	__in ULONG DeviceId
	)
{
	return FALSE;
}

PVDISK_OBJECT MntGetDeviceById(
	__in ULONG DeviceId
	)
{
	SINGLE_LIST_ENTRY *Node;
	PVDISK_OBJECT DiskObj;

	if (MntDiskList.Next == NULL)
		return NULL;

	for (Node = MntDiskList.Next; Node != NULL ; Node = Node->Next)
	{
		DiskObj = CONTAINING_RECORD(Node,VDISK_LIST_ENTRY,Entry)->VDisk;

		if (DiskObj->DeviceId == DeviceId)
			return DiskObj;
	}

	return NULL;
}

#define MntGetNextDeviceIdentifier() (MntDeviceCount + 1)
#define MntPrepareNextDeviceId() (++MntDeviceCount)

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
	ULONG DevId;
	PVDISK_OBJECT DiskObj;

	MntAcquireGeneralLock();

	DevId = MntGetNextDeviceIdentifier();

	Status = VdiAllocateVirtualDisk(MntDriverObject,DevId,DiskLength,&DiskObj);

	if (!NT_SUCCESS(Status))
	{
		Status = STATUS_UNSUCCESSFUL;
		goto Exit;
	}

	MntPrepareNextDeviceId();

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
	NTSTATUS Status;
	PVDISK_OBJECT Disk;
	MntAcquireGeneralLock();

	Disk = MntGetDeviceById(((PDEVICE_EXTENSION_DATA)DeviceInfo)->DeviceId);

	if (Disk == NULL)
	{
		Status = STATUS_DEVICE_DOES_NOT_EXIST;
		Irp->IoStatus.Status = Status;
		Irp->IoStatus.Information = 0;
		IofCompleteRequest(Irp,IO_NO_INCREMENT);
		MntReleaseGeneralLock();
		return Status;
	}

	MntReleaseGeneralLock();

	Status = VdiDispatchDiskIrp(Disk,Irp);

	return Status;
}