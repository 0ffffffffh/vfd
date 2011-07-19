#ifndef __SDKBASE_H__
#define __SDKBASE_H__

#define VFDAPI __stdcall

typedef short VFDSTATUS;

/*
 S             CODE
--+------------------------------
|1|1 1 1 1 1 1 1 1 1 1 1 1 1 1 1| 
--+------------------------------
*/

#define SUCCESS_BIT						0x0000
#define FAILED_BIT						0x8000

#define MAX_STATUS_CODE					0x7FFF

#define DEVICE_CODE_BASE				0x1000
#define IO_CODE_BASE					0x2000

#define VFDC_DEVICE_CREATE				(DEVICE_CODE_BASE + 0)
#define VFDC_DEVICE_DELETE				(DEVICE_CODE_BASE + 1)

#define VFDC_OPER_WRITE					(IO_CODE_BASE + 0)
#define VFDC_OPER_READ					(IO_CODE_BASE + 1)


#define MAKEVFDSTATUS(Success, Code)	((Success) | Code)
#define VFD_SUCCESS(Code)				((Code) >= 0)
#define VFD_CODE(Status)				((((VFDSTATUS)Status) & MAX_CODE) )

#define INVALID_VDISK_ID ((ULONG)-1)


#include "vfdstatcodes.h"


#endif //__SDKBASE_H__