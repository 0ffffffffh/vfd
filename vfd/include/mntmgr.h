#include "ntvfd.h"

NTSTATUS MntInitializeMountMgr(
	__in PDRIVER_OBJECT DriverObject
	);

NTSTATUS MntMountDisk(
	__in ULONG64 DiskLength,
	__out PULONG VdiskId
	);

NTSTATUS MntUnmountDisk(
	__in ULONG VdiskId
	);

NTSTATUS MntDispatchDiskIrp(
	__in PVOID DeviceInfo,
	__in PIRP Irp
	);