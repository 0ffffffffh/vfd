#ifndef __NTVFD_H__
#define __NTVFD_H__

#include <ntifs.h>

#define VFD_DEVICE_NAME L"\\Device\\VfdDevController"
#define VFD_ROOT_DIR_DEV L"\\Device\\VfdDevRoot"
#define VFD_VDISK_DEV_FORMAT VFD_ROOT_DIR_DEV L"\\vdisk%d"

#define KdbPrint(format,...) DbgPrint(format,__VA_ARGS__)

#define NTFAILMSG(status) DbgPrint("Failed NTSTATUS (0x%x) in %s at %d",status,__FUNCTION__,__LINE__)
#define NTFAILMSGEX(status, extraformat,...) DbgPrint("Failed NTSTATUS (0x%x) in %s at %d - " extraformat, __FUNCTION__,__LINE__,__VA_ARGS__)

#define NOTIMPLEMENTED() DbgPrint("%s is not implemented!",__FUNCTION__)

#define IRQLCHECK(exp,bsod) \
{ \
	if (!(exp)) \
	{ \
		KdbPrint("IRQL assertion failed (%s:%d)", __FUNCTION__,__LINE__); \
		if ((bsod)>0) \
			KeBugCheck(bsod); \
	} \
} \


typedef struct _DEVICE_EXTENSION_DATA
{
	ULONG DeviceId;
}DEVICE_EXTENSION_DATA,*PDEVICE_EXTENSION_DATA;

#endif //__NTVFD_H__