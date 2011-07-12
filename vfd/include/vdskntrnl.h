

#ifndef __VDSKNTRNL_H__
#define __VDSKNTRNL_H__

#include "ntvfd.h"

typedef struct _IOPARAMETER
{
	UCHAR Major;
	ULONG Length;
	ULONG64 Offset;
	PUCHAR IoBuffer;
	union {
		ULONG_PTR r0;
		ULONG_PTR r1;
		ULONG_PTR r2;
		ULONG_PTR r3;
	}Reserved;
}IOPARAMETER,*PIOPARAMETER;

#endif //__VDSKNTRNL_H__