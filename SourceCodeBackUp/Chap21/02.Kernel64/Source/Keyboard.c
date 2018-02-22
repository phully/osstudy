#include "Types.h"
#include "AssemblyUtility.h"
#include "Keyboard.h"
#include "Queue.h"
#include "Utility.h"
#include "Synchronization.h"

BOOL kIsOutputBufferFull(void)
{
    if(kInPortByte(0x64) & 0x01)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL kIsInputBufferFull(void)
{
    if(kInPortByte(0x64) & 0x02)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL kWaitForACKAndPutOtherScanCode(void)
{
    int i,j;
    BYTE bData;
    BOOL bResult = FALSE;

    for(j=0;j<100;j++)
    {
        for(i=0;i<0xFFFF;i++)
        {
            if(kIsOutputBufferFull() == TRUE)
            {
                break;
            }
        }

        bData = kInPortByte(0x60);

        if(bData == 0xFA)
        {
            bResult = TRUE;
            break;
        }
        else
        {
            kConvertScanCodeAndPutQueue(bData);
        }
    }
    return bResult;
}

BOOL kActivateKeyboard(void)
{
    int i,j;
    BOOL bPreviousInterrupt;
    BOOL bResult;

    bPreviousInterrupt = kSetInterruptFlag(FALSE);

    kOutPortByte(0x64,0xAE);

    for(i=0;i<0xFFFF;i++)
    {
        if(kIsInputBufferFull() == FALSE)
        {
            break;
        }    
    }

    kOutPortByte(0x60,0xF4);

    bResult = kWaitForACKAndPutOtherScanCode();

    kSetInterruptFlag(bPreviousInterrupt);
    return bResult;
}

BYTE kGetKeyboardScanCode(void)
{
    while(kIsOutputBufferFull() == FALSE)
    {
        ;
    }

    return kInPortByte(0x60);
}

BOOL kChangeKeyboardLED(BOOL bCapsLockOn, BOOL bNumLockOn,BOOL bScrollLockOn)
{
    int i,j;
    BOOL bPreviousInterrupt;
    BOOL bResult;
    BYTE bData;

    bPreviousInterrupt = kSetInterruptFlag(FALSE);

    for(i=0;i<0xFFFF;i++)
    {
        if(kIsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    kOutPortByte(0x60,0xED);

    for(i=0;i<0xFFFF;i++)
    {
        if(kIsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    bResult = kWaitForACKAndPutOtherScanCode();

    if(bResult == FALSE)
    {
        kSetInterruptFlag(bPreviousInterrupt);
        return FALSE;
    }

    kOutPortByte(0x60,(bCapsLockOn << 2) | (bNumLockOn << 1) | bScrollLockOn);

    for(i=0;i<0xFFFF;i++)
    {
        if(kIsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    bResult = kWaitForACKAndPutOtherScanCode();
    
    kSetInterruptFlag(bPreviousInterrupt);
    return bResult;
}

void kEnableA20Gate(void)
{
    BYTE bOutputPortData;
    int i;

    kOutPortByte(0x64,0xD0);

    for(i=0;i<0xFFFF;i++)
    {
        if(kIsOutputBufferFull() == TRUE)
        {
            break;
        }
    }

    bOutputPortData = kInPortByte(0x60);

    bOutputPortData |= 0x01;

    for(i=0;i<0xFFFF;i++)
    {
        if(kIsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    kOutPortByte(0x64,0xD1);
    kOutPortByte(0x60,bOutputPortData);
}

void kReboot(void)
{
    int i;

    for(i=0;i<0xFFFF;i++)
    {
        if(kIsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    kOutPortByte(0x64,0xD1);
    kOutPortByte(0x60,0x00);

    while(1)
    {
        ;
    }
}

static KEYBOARDMANAGER gs_stKeyboardManager = {0,};

static QUEUE gs_stKeyQueue;
static KEYDATA gs_vstKeyQueueBuffer[KEY_MAXQUEUECOUNT];

static KEYMAPPINGENTRY gs_vstKeyMappingTable[KEY_MAPPINGTABLEMAXCOUNT] = 
{
    {KEY_NONE,KEY_NONE},
    {KEY_ESC,KEY_ESC},
    {'1','!'},
    {'2','@'},
    {'3','#'},
    {'4','$'},
    {'5','%'},
    {'6','^'},
    {'7','&'},
    {'8','*'},
    {'9','('},
    {'0',')'},
    {'-','_'},
    {'=','+'},
    {KEY_BACKSPACE,KEY_BACKSPACE},
    {KEY_TAB,KEY_TAB},
    {'q','Q'},
    {'w','W'},
    {'e','E'},
    {'r','R'},
    {'t','T'},
    {'y','Y'},
    {'u','U'},
    {'i','I'},
    {'o','O'},
    {'p','P'},
    {'[','{'},
    {']','}'},
    {'\n','\n'},
    {KEY_CTRL,KEY_CTRL},
    {'a','A'},
    {'s','S'},
    {'d','D'},
    {'f','F'},
    {'g','G'},
    {'h','H'},
    {'j','J'},
    {'k','K'},
    {'l','L'},
    {';',':'},
    {'\'','\"'},
    {'`','~'},
    {KEY_LSHIFT,KEY_LSHIFT},
    {'\\','|'},
    {'z','Z'},
    {'x','X'},
    {'c','C'},
    {'v','V'},
    {'b','B'},
    {'n','N'},
    {'m','M'},
    {',','<'},
    {'.','>'},
    {'/','?'},
    {KEY_RSHIFT,KEY_RSHIFT},
    {'*','*'},
    {KEY_LALT,KEY_LALT},
    {' ',' '},
    {KEY_CAPSLOCK,KEY_CAPSLOCK},
    {KEY_F1,KEY_F1},
    {KEY_F2,KEY_F2},
    {KEY_F3,KEY_F3},
    {KEY_F4,KEY_F4},
    {KEY_F5,KEY_F5},
    {KEY_F6,KEY_F6},
    {KEY_F7,KEY_F7},
    {KEY_F8,KEY_F8},
    {KEY_F9,KEY_F9},
    {KEY_F10,KEY_F10},
    {KEY_NUMLOCK,KEY_NUMLOCK},
    {KEY_SCROLLLOCK,KEY_SCROLLLOCK},
    {KEY_HOME,'7'},
    {KEY_UP,'8'},
    {KEY_PAGEUP,'9'},
    {'-','-'},
    {KEY_LEFT,'4'},
    {KEY_CENTER,'5'},
    {KEY_RIGHT,'6'},
    {'+','+'},
    {KEY_END,'1'},
    {KEY_DOWN,'2'},
    {KEY_PAGEDOWN,'3'},
    {KEY_INS,'0'},
    {KEY_DEL,'.'},
    {KEY_NONE,KEY_NONE},
    {KEY_NONE,KEY_NONE},
    {KEY_NONE,KEY_NONE},
    {KEY_F11,KEY_F11},
    {KEY_F12,KEY_F12}
};

BOOL kIsAlphabetScanCode(BYTE bScanCode)
{
    if(( 'a' <= gs_vstKeyMappingTable[bScanCode].bNormalCode) && (gs_vstKeyMappingTable[bScanCode].bNormalCode <= 'z'))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL kIsNumberOrSymbolScanCode(BYTE bScanCode)
{
    if(( 2 <= bScanCode ) && (bScanCode <= 53) && (kIsAlphabetScanCode(bScanCode) == FALSE))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL kIsNumberPadScanCode(BYTE bScanCode)
{
    if((71 <= bScanCode) && (bScanCode<=83))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL kIsUseCombinedCode(BOOL bScanCode)
{
    BYTE bDownScanCode;
    BOOL bUseCombinedKey;

    bDownScanCode = bScanCode & 0x7F;

    if(kIsAlphabetScanCode(bDownScanCode) == TRUE)
    {
        if(gs_stKeyboardManager.bShiftDown ^ gs_stKeyboardManager.bCapsLockOn)
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }
    else if(kIsNumberOrSymbolScanCode(bDownScanCode) == TRUE)
    {
        if(gs_stKeyboardManager.bShiftDown == TRUE)
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }
    else if((kIsNumberPadScanCode(bDownScanCode)==TRUE) && (gs_stKeyboardManager.bExtendedCodeIn == FALSE))
    {
        if(gs_stKeyboardManager.bNumLockOn == TRUE)
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }

    return bUseCombinedKey;
}

void UpdateCombinationKeyStatusAndLED(BYTE bScanCode)
{
    BOOL bDown;
    BYTE bDownScanCode;
    BOOL bLEDStatusChanged = FALSE;

    if(bScanCode & 0x80)
    {
        bDown = FALSE;
        bDownScanCode = bScanCode & 0x7F;
    }
    else
    {
        bDown = TRUE;
        bDownScanCode = bScanCode;
    }

    if((bDownScanCode == 42) || (bDownScanCode == 54))
    {
        gs_stKeyboardManager.bShiftDown = bDown;
    }
    else if((bDownScanCode == 58) && (bDown == TRUE))
    {
        gs_stKeyboardManager.bCapsLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }
    else if((bDownScanCode == 69) && (bDown == TRUE))
    {
        gs_stKeyboardManager.bNumLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }
    else if((bDownScanCode == 70) && (bDown == TRUE))
    {
        gs_stKeyboardManager.bScrollLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }

    if(bLEDStatusChanged == TRUE)
    {
        kChangeKeyboardLED(gs_stKeyboardManager.bCapsLockOn,gs_stKeyboardManager.bNumLockOn,gs_stKeyboardManager.bScrollLockOn);
    }
}

BOOL kConvertScanCodeToASCIICode(BYTE bScanCode,BYTE *pbASCIICode,BOOL *pbFlags)
{
    BOOL bUseCombinedKey;

    if(gs_stKeyboardManager.iSkipCountForPause > 0)
    {
        gs_stKeyboardManager.iSkipCountForPause--;
        return FALSE;
    }

    if(bScanCode == 0xE1)
    {
        *pbASCIICode = KEY_PAUSE;
        *pbFlags = KEY_FLAGS_DOWN;
        gs_stKeyboardManager.iSkipCountForPause = KEY_SKIPCOUNTFORPAUSE;
        return TRUE;
    }
    else if(bScanCode == 0xE0)
    {
        gs_stKeyboardManager.bExtendedCodeIn = TRUE;
        return FALSE;
    }

    bUseCombinedKey = kIsUseCombinedCode(bScanCode);

    if(bUseCombinedKey == TRUE)
    {
        *pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bCombinedCode;
    }
    else
    {
        *pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bNormalCode;
    }

    if(gs_stKeyboardManager.bExtendedCodeIn == TRUE)
    {
        *pbFlags = KEY_FLAGS_EXTENDEDKEY;
        gs_stKeyboardManager.bExtendedCodeIn = FALSE;
    }
    else
    {
        *pbFlags = 0;
    }

    if((bScanCode & 0x80) == 0)
    {
        *pbFlags |= KEY_FLAGS_DOWN;
    }

    UpdateCombinationKeyStatusAndLED(bScanCode);
    return TRUE;
}

BOOL kInitializeKeyboard(void)
{
    kInitializeQueue(&gs_stKeyQueue,gs_vstKeyQueueBuffer,KEY_MAXQUEUECOUNT,sizeof(KEYDATA));

    return kActivateKeyboard();
}

BOOL kConvertScanCodeAndPutQueue(BYTE bScanCode)
{
    KEYDATA stData;
    BOOL bResult = FALSE;
    BOOL bPreviousInterrupt;

    stData.bScanCode = bScanCode;

    if(kConvertScanCodeToASCIICode(bScanCode,&(stData.bASCIICode),&(stData.bFlags)) == TRUE)
    {
        bPreviousInterrupt = kLockForSystemData();

        bResult = kPutQueue(&gs_stKeyQueue,&stData);

        kUnlockForSystemData(bPreviousInterrupt);
    }

    return bResult;
}

BOOL kGetKeyFromKeyQueue(KEYDATA *pstData)
{
    BOOL bResult;
    BOOL bPreviousInterrupt;

    if(kIsQueueEmpty(&gs_stKeyQueue) == TRUE)
    {
        return FALSE;
    }

    bPreviousInterrupt = kLockForSystemData();

    bResult = kGetQueue(&gs_stKeyQueue,pstData);

    kUnlockForSystemData(bPreviousInterrupt);
    return bResult;
}