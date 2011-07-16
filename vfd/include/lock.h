
#ifndef __LOCK_H__
#define __LOCK_H__

#define KeAcquireMutexLock(Mutex) KeWaitForSingleObject(Mutex,Executive,KernelMode,FALSE,NULL)
#define KeReleaseMutexLock(Mutex) KeReleaseMutex(Mutex,FALSE);

#endif //__LOCK_H__