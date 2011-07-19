//FLAT API WRAPPER CLASS for C++

#ifndef __DRIVERCONTROL_H__
#define __DRIVERCONTROL_H__

class VirtualDisk;

class DriverControl
{
private:
	HANDLE Driver;
	VirtualDisk *Disk;

	bool ConnectDriver();
	void DisconnectDriver();

public:
	DriverControl(VirtualDisk *VDisk);
	~DriverControl(void);

	bool IsConnected() const;
};

#endif //__DRIVERCONTROL_H__