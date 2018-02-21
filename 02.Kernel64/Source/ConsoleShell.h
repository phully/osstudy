#ifndef __CONSOLESHELL_H__
#define __CONSOLESHELL_H__

#include "Types.h"
#include "Task.h"

#define CONSOLESHELL_MAXCOMMANDBUFFERCOUNT  300
#define CONSOLESHELL_PROMPTMESSAGE          "MINT64>"

typedef void (*CommandFunction)(const char *pcParameter);

#pragma pack(push,1)

typedef struct kShellCommandEntryStruct
{
    char *pcCommand;
    char *pcHelp;
    CommandFunction pfFunction;
}SHELLCOMMANDENTRY;

typedef struct kParameterListStruct
{
    const char *pcBuffer;
    int iLength;
    int iCurrentPosition;
}PARAMETERLIST;

#pragma pack(pop)

void kStartConsoleShell(void);
void kExecuteCommand(const char *pcCommandBuffer);
void kInitializeParameter(PARAMETERLIST *pstList,const char *pcParameter);
int kGetNextParameter(PARAMETERLIST *pstList,char *pcParameter);

void kHelp(const char *pcParameterBuffer);
void kCls(const char *pcParameterBuffer);
void kShowTotalRAMSize(const char *pcParameterBuffer);
void kStringToDecimalHexTest(const char *pcParameterBuffer);
void kShutDown(const char *pcParameterBuffer);
void kSetTimer(const char *pcParameterBuffer);
void kWaitUsingPIT(const char *pcParameterBuffer);
void kReadTimeStampCounter(const char *pcParameterBuffer);
void kMeasureProcessorSpeed(const char *pcParameterBuffer);
void kShowDateAndTime(const char *pcParameterBuffer);

void kTestTask(void);
void kCreateTestTask(const char *pcParameterBuffer);
static void kChangeTaskPriority(const char *pcParameterBuffer);
static void kShowTaskList(const char *pcParameterBuffer);
static void kKillTask(const char *pcParameterBuffer);
static void kCPULoad(const char *pcParameterBuffer);

static void kTestMutex(const char *pcParameterBuffer);
static void kTestThread(const char *pcParameterBuffer);

#endif