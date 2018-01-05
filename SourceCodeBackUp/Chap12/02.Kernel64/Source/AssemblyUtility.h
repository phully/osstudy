#ifndef __ASSEMBLY_UTILITY__
#define __ASSEMBLY_UTILITY__

#include "Types.h"

BYTE kInPortByte(WORD wPort);
void kOutPortByte(WORD wPort,WORD bData);
void kLoadGDTR(QWORD qwGDTRAddress);
void kLoadTR(WORD wTSSSegmentOffset);
void kLoadIDTR(QWORD qwIDTRAddress);

#endif