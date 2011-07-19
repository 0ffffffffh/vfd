
#include "..\include\sdk.h"

VFDSTATUS VFDAPI VfdCreateVirtualDisk(
	__in LARGE_INTEGER DiskSize,
	__in DiskType Type,
	__out PULONG DiskId
	)
{
	return VFD_STATUS_NOT_IMPLEMENTED;
}

VFDSTATUS VFDAPI VfdCreateVirtualDiskEx(
	__in		LARGE_INTEGER Size,
	__in		DiskType Type,
	__in		BOOL Persistent,
	__in_opt	LPWSTR PhysicalMapFile,
	__out		PULONG DiskId
	)
{
	return VFD_STATUS_NOT_IMPLEMENTED;
}

VFDSTATUS VFDAPI VfdDeleteVirtualDisk(
	__in ULONG DiskId
	)
{
	return VFD_STATUS_NOT_IMPLEMENTED;
}