#ifndef __SDK_H__
#define __SDK_H__

#include "sdkbase.h"
#include "vfdtypes.h"

#if !defined(_WIN32_WINNT)
#include <Windows.h>
#endif

//FLAT API for C

VFDSTATUS VFDAPI VfdCreateVirtualDisk(
	__in LARGE_INTEGER DiskSize,
	__in DiskType Type,
	__out PULONG DiskId
	);

VFDSTATUS VFDAPI VfdCreateVirtualDiskEx(
	__in		LARGE_INTEGER Size,
	__in		DiskType Type,
	__in		BOOL Persistent,
	__in_opt	LPWSTR PhysicalMapFile,
	__out		PULONG DiskId
	);

VFDSTATUS VFDAPI VfdDeleteVirtualDisk(
	__in ULONG DiskId
	);



#endif //__SDK_H__