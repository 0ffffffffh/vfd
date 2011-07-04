#include "..\include\mntmgr.h"
#include "..\include\vdisk.h"
#include "..\include\memory.h"

KMUTEX MntGeneralMountLock;
PDRIVER_OBJECT MntDriverObject;
ULONG MntDeviceCount=0;
LIST_ENTRY MntDiskList;

typedef struct _VDISK_LIST_ENTRY
{
	LIST_ENTRY Entry;
	PVDISK_OBJECT VDisk;
}VDISK_LIST_ENTRY,*PVDISK_LIST_ENTRY;



#define MntAcquireGeneralLock() KeWaitForSingleObject(&MntGeneralMountLock,Executive,KernelMode,FALSE,NULL)

#define MntReleaseGeneralLock() KeReleaseMutex(&MntGeneralMountLock,FALSE)

#define MntIsRegistered(Vdisk) \
{ \
	((BOOLEAN)Vdisk->ContainedNode != NULL); \
} \


VFDINTERNAL VOID MntRegisterDiskDeviceObject(
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
	
	InsertTailList(&MntDiskList,&(ListEntry->Entry));
	
	Vdisk->ContainedNode = &ListEntry->Entry;	
}

VFDINTERNAL BOOLEAN _MntDeRegisterDiskDeviceObject(
	__in PLIST_ENTRY Node
	)
{
	PVDISK_LIST_ENTRY Entry;

	if (Node == NULL)
		return FALSE;

	Entry = CONTAINING_RECORD(Node,VDISK_LIST_ENTRY,Entry);

	RemoveEntryList(Node);

	MemFreePoolMemory(Entry);

	return TRUE;
}

VFDINTERNAL BOOLEAN MntDeRegisterDiskDeviceObject(
	__in PVDISK_OBJECT DiskDev
	)
{
	if (DiskDev == NULL)
		return FALSE;

	return _MntDeRegisterDiskDeviceObject(DiskDev->ContainedNode);
}

VFDINTERNAL PLIST_ENTRY MntGetDeviceNodeById(
	__in ULONG DeviceId
	)
{
	LIST_ENTRY *Node;

	if (IsListEmpty(&MntDiskList))
		return NULL;

	for (Node = MntDiskList.Flink; Node != NULL; Node = Node->Flink)
	{
		if (CONTAINING_RECORD(Node,VDISK_LIST_ENTRY,Entry)->VDisk->DeviceId == DeviceId)
			return Node;
	}

	return NULL;
}

VFDINTERNAL PVDISK_OBJECT MntGetDeviceById(
	__in ULONG DeviceId
	)
{
	LIST_ENTRY *Node;

	Node = MntGetDeviceNodeById(DeviceId);

	if (!Node)
		return NULL;

	return CONTAINING_RECORD(Node,VDISK_LIST_ENTRY,Entry)->VDisk;
}

VFDINTERNAL NTSTATUS FASTCALL _MntUnmountDisk(
	__in PVDISK_OBJECT Disk
	);

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
	
	InitializeListHead(&MntDiskList);

	RtlInitUnicodeString(&DevDirName,VFD_ROOT_DIR_DEV);

	InitializeObjectAttributes(&ObjAttrib,&DevDirName,OBJ_OPENIF,NULL,NULL);

	Status = ZwCreateDirectoryObject(&DevObjHandle, DIRECTORY_ALL_ACCESS, &ObjAttrib);

	if (!NT_SUCCESS(Status))
		return Status;

	MntDriverObject = DriverObject;

	return STATUS_SUCCESS;
}

NTSTATUS MntUninitializeMountMgr(
	)
{
	LIST_ENTRY *Node, *Temp;
	BOOLEAN Success;

	if (IsListEmpty(&MntDiskList))
		return STATUS_UNSUCCESSFUL;


	MntAcquireGeneralLock();

	Node = MntDiskList.Blink;

	if (MntDriverObject != NULL)
	{
		while (Node != NULL)
		{
			Temp = Node->Blink;
			_MntUnmountDisk(CONTAINING_RECORD(Node,VDISK_LIST_ENTRY,Entry)->VDisk);
			Node = Temp;
		}
	}

	MntReleaseGeneralLock();

	return STATUS_UNSUCCESSFUL;
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

VFDINTERNAL NTSTATUS FASTCALL _MntUnmountDisk(
	__in PVDISK_OBJECT Disk
	)
{
	if (Disk == NULL)
		return STATUS_DEVICE_DOES_NOT_EXIST;

	MntDeRegisterDiskDeviceObject(Disk);

	return VdiFreeVirtualDisk(Disk);
}

NTSTATUS MntUnmountDisk(
	__in ULONG VdiskId
	)
{
	NTSTATUS Status;
	PVDISK_OBJECT Disk;
	MntAcquireGeneralLock();

	Disk = MntGetDeviceById(VdiskId);
	
	Status = _MntUnmountDisk(Disk);

	MntReleaseGeneralLock();

	return Status;
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