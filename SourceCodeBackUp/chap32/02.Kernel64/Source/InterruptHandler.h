#ifndef __INTERRUPTHANDLER_H__
#define __INTERRUPTHANDLER_H__

#include "Types.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"
#include "HardDisk.h"
#include "InterruptHandler.h"
#include "MultiProcessor.h"

#define INTERRUPT_MAXVECTORCOUNT        16
#define INTERRUPT_LOADBALANCINGDIVIDOR  10

typedef struct kInterruptManagerStruct
{
    QWORD vvqwCoreInterruptCount[MAXPROCESSORCOUNT][INTERRUPT_MAXVECTORCOUNT];
    BOOL bUseLoadBalancing;
    BOOL bSymmetricIOMode;
}INTERRUPTMANAGER;

void kSetSymmetricIOMode(BOOL bSymmetricIOMode);
void kSetInterruptLoadBalancing(BOOL bUseLoadBalancing);
void kIncreaseInterruptCount(int iIRQ);
void kSendEOI(int iIRQ);
INTERRUPTMANAGER *kGetInterruptManager(void);
void kProcessLoadBalancing(int iIRQ);

void kCommonExceptionHandler(int iVectorNumber,QWORD qwErrorCode);
void kCommonInterruptHandler(int iVectorNumber);
void kKeyboardHandler(int iVectorNumber);
void kTimerHandler(int iVectorNumber);
void KDeviceNotAvailableHandler(int iVectorNumber);
void kHDDHandler(int iVectorNumber);

#endif