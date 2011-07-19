#include "StdAfx.h"
#include "DriverControl.h"
#include "VirtualDisk.h"

DriverControl::DriverControl(VirtualDisk *VDisk) : 
	Driver(INVALID_HANDLE_VALUE),
	Disk(VDisk)
{
	ConnectDriver();
}

DriverControl::~DriverControl(void)
{
	DisconnectDriver();
}

bool DriverControl::IsConnected() const
{
	return Driver != INVALID_HANDLE_VALUE;
}

bool DriverControl::ConnectDriver()
{
	if (Disk == NULL || !Disk->IsOnline())
		return false;



	return true;
}

void DriverControl::DisconnectDriver()
{
}