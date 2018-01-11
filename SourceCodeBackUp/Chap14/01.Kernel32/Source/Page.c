#include "Page.h"

void kInitializePageTables(void)
{
    PML4ENTRY *pstPML4Entry;
    PDPTENTRY *pstPDPTEntry;
    PDENTRY *pstPDEntry;
    DWORD dwMappingAddress;
    int i;

    pstPML4Entry = (PML4ENTRY *)0x100000;
    kSetPageEntryData( &(pstPML4Entry[0]),0x00,0x101000,PAGE_FLAGS_DEFAULT,0);
    for(i=1;i<512;i++)
    {
        kSetPageEntryData(&(pstPML4Entry[i]),0,0,0,0);
    }

    pstPDPTEntry = (PDPTENTRY *)0x101000;
    for(i=0;i<64;i++)
    {
        kSetPageEntryData(&(pstPDPTEntry[i]),0,0x102000+(i*PAGE_TABLESIZE),PAGE_FLAGS_DEFAULT,0);
    }
    for(i=64;i<PAGE_MAXENTRYCOUNT;i++)
    {
        kSetPageEntryData(&(pstPDPTEntry[i]),0,0,0,0);
    }

    pstPDEntry = (PDENTRY *)0x102000;
    dwMappingAddress = 0;
    for(i=0;i<PAGE_MAXENTRYCOUNT*64;i++)
    {
        kSetPageEntryData(&(pstPDEntry[i]),(i*(PAGE_DEFAULTSIZE >> 20))>>12,\
        dwMappingAddress,PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS,0);
        dwMappingAddress += PAGE_DEFAULTSIZE;
    }
}

void kSetPageEntryData(PTENTRY *pstEntry,DWORD dwUpperBaseAddress,DWORD dwLowerBaseAddress, \
    DWORD dwLowerFlags,DWORD dwUpperFlags)
{
    pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
    pstEntry->dwUpperBaseAddressAndEXB = (dwUpperBaseAddress & 0xFF) | dwUpperFlags;
}