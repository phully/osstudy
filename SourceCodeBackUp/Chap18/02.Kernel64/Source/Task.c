#include "Task.h"
#include "Descriptor.h"

static SCHEDULER gs_stScheduler;
static TCBPOOLMANAGER gs_stTCBPoolManager;

void kInitializeTCBPool(void)
{
    int i;

    kMemSet(&(gs_stTCBPoolManager),0,sizeof(gs_stTCBPoolManager));

    gs_stTCBPoolManager.pstStartAddress = (TCB *)TASK_TCBPOOLADDRESS;
    kMemSet(TASK_TCBPOOLADDRESS,0,sizeof(TCB)*TASK_MAXCOUNT);

    for(i=0;i<TASK_MAXCOUNT;i++)
    {
        gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;
    }

    gs_stTCBPoolManager.iMaxCount = TASK_MAXCOUNT;
    gs_stTCBPoolManager.iAllocatedCount = 1;
}

TCB *kAllocateTCB(void)
{
    TCB *pstEmptyTCB;
    int i;

    if(gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount)
    {
        return NULL;
    }

    for(i=0;i<gs_stTCBPoolManager.iMaxCount;i++)
    {
        if((gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID >> 32) == 0)
        {
            pstEmptyTCB = &(gs_stTCBPoolManager.pstStartAddress[i]);
            break;
        }
    }

    pstEmptyTCB->stLink.qwID = ((QWORD)gs_stTCBPoolManager.iAllocatedCount << 32) | i;
    gs_stTCBPoolManager.iUseCount++;
    gs_stTCBPoolManager.iAllocatedCount++;

    if(gs_stTCBPoolManager.iAllocatedCount == 0)
    {
        gs_stTCBPoolManager.iAllocatedCount = 1;
    }

    return pstEmptyTCB;
}


void kFreeTCB(QWORD qwID)
{
    int i;

    i = qwID & 0xFFFFFFFF;

    kMemSet(&(gs_stTCBPoolManager.pstStartAddress[i].stContext),0,sizeof(CONTEXT));
    gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;

    gs_stTCBPoolManager.iUseCount--;
}

TCB* kCreateTask(QWORD qwFlags,QWORD qwEntryPointAddress)
{
    TCB *pstTask;
    void *pvStackAddress;

    pstTask = kAllocateTCB();
    if(pstTask == NULL)
    {
        return NULL;
    }


    pvStackAddress = (void *)(TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * (pstTask->stLink.qwID & 0xFFFFFFFF)));

    kSetUpTask(pstTask,qwFlags,qwEntryPointAddress,pvStackAddress,TASK_STACKSIZE);
    kAddTaskToReadyList(pstTask);

    return pstTask;
}

void kSetUpTask(TCB *pstTCB,QWORD qwFlags,QWORD qwEntryPointAddress,void *pvStackAddress,QWORD qwStackSize)
{
    kMemSet(pstTCB->stContext.vqRegister,0,sizeof(pstTCB->stContext.vqRegister));

    pstTCB->stContext.vqRegister[TASK_RSPOFFSET] = (QWORD)pvStackAddress + qwStackSize;
    pstTCB->stContext.vqRegister[TASK_RBPOFFSET] = (QWORD)pvStackAddress + qwStackSize;

    
    pstTCB->stContext.vqRegister[TASK_CSOFFSET] = GDT_KERNELCODESEGMENT;
    pstTCB->stContext.vqRegister[TASK_DSOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_ESOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_FSOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_GSOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_SSOFFSET] = GDT_KERNELDATASEGMENT;

    pstTCB->stContext.vqRegister[TASK_RIPOFFSET] = qwEntryPointAddress;
    pstTCB->stContext.vqRegister[TASK_RFLAGSOFFSET] |= 0x0200;

    pstTCB->pvStackAddress = pvStackAddress;
    pstTCB->qwStackSize = qwStackSize;
    pstTCB->qwFlags = qwFlags;
}

void kInitializeScheduler(void)
{
    kInitializeTCBPool();

    kInitializeList(&(gs_stScheduler.stReadyList));

    gs_stScheduler.pstRunningTask = kAllocateTCB();
}

void kSetRunningTask(TCB *pstTask)
{
    gs_stScheduler.pstRunningTask = pstTask;
}

TCB *kGetRunningTask(void)
{
    return gs_stScheduler.pstRunningTask;
}

TCB *kGetNextTaskToRun(void)
{
    if(kGetListCount(&(gs_stScheduler.stReadyList)) == 0)
    {
        return NULL;
    }

    return (TCB *)kRemoveListFromHeader(&(gs_stScheduler.stReadyList));
}

void kAddTaskToReadyList(TCB *pstTask)
{
    kAddListToTail(&(gs_stScheduler.stReadyList),pstTask);
}

void kSchedule(void)
{
    TCB *pstRunningTask, *pstNextTask;
    BOOL bPreviousFlag;

    if(kGetListCount(&(gs_stScheduler.stReadyList)) == 0)
    {
        return ;
    }

    bPreviousFlag = kSetInterruptFlag(FALSE);

    pstNextTask = kGetNextTaskToRun();
    if(pstNextTask == NULL)
    {
        kSetInterruptFlag(bPreviousFlag);
        return ;
    }

    pstRunningTask = gs_stScheduler.pstRunningTask;
    kAddTaskToReadyList(pstRunningTask);

    gs_stScheduler.pstRunningTask = pstNextTask;
    kSwitchContext(&(pstRunningTask->stContext),&(pstNextTask->stContext));

    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

    kSetInterruptFlag(bPreviousFlag);
}

BOOL kScheduleInInterrupt(void)
{
    TCB *pstRunningTask,*pstNextTask;
    char *pcContextAddress;

    pstNextTask = kGetNextTaskToRun();

    if(pstNextTask == NULL)
    {
        return FALSE;
    }

    pcContextAddress = (char *)IST_STARTADDRESS + IST_SIZE - sizeof(CONTEXT);

    pstRunningTask = gs_stScheduler.pstRunningTask;
    kMemCpy(&(pstRunningTask->stContext),pcContextAddress,sizeof(CONTEXT));
    kAddTaskToReadyList(pstRunningTask);

    gs_stScheduler.pstRunningTask = pstNextTask;
    kMemCpy(pcContextAddress,&(pstNextTask->stContext),sizeof(CONTEXT));

    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
    return TRUE;
}

void kDecreaseProcessorTime(void)
{
    if(gs_stScheduler.iProcessorTime > 0)
    {
        gs_stScheduler.iProcessorTime--;
    }
}

BOOL kIsProcessorTimeExpired(void)
{
    if(gs_stScheduler.iProcessorTime <= 0)
    {
        return TRUE;
    }
    return FALSE;
}