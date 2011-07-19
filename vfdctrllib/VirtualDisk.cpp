#include "StdAfx.h"
#include "VirtualDisk.h"
#include "include\sdk.h"

VirtualDisk::VirtualDisk(void)
{
	Id = INVALID_VDISK_ID;
}


VirtualDisk::~VirtualDisk(void)
{
	if (Id != INVALID_VDISK_ID)
	{
		VfdDeleteVirtualDisk(Id);
	}
}

bool VirtualDisk::IsOnline() const
{
	return (Id != INVALID_VDISK_ID);
}

bool VirtualDisk::Create(ULONG64 Size, DiskType Type, bool Persistent, LPWSTR PhysicalMapFile)
{
	VFDSTATUS Status;
	LARGE_INTEGER LSize;
	
	LSize.QuadPart = Size;

	Status = VfdCreateVirtualDiskEx(LSize,Type,Persistent ? TRUE : FALSE,PhysicalMapFile,&Id);

	return VFD_SUCCESS(Status) == true;
}