//FLAT API WRAPPER CLASS for C++

#ifndef __VIRTUALDISK_H__
#define __VIRTUALDISK_H__

#include ".\include\vfdtypes.h"

class VirtualDisk
{
private:
	ULONG Id;

public:
	VirtualDisk(void);
	~VirtualDisk(void);

	bool IsOnline() const;
	bool Create(ULONG64 Size, DiskType Type, bool Persistent = false, LPWSTR PhysicalMapFile = NULL);
	
};

#endif //__VIRTUALDISK_H__