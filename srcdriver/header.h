#pragma once

#include <ntifs.h>
#include <ntddkbd.h>
#include <intrin.h>

#define XOR_KEY 0xDEADBEEF1337C0DEull
#define POOL_TAG 0x564F5858

#define REQUEST_TABLE_SIZE 64
#define REQUEST_ENTRY_SIZE 4176
#define RESPONSE_AREA_OFFSET 267288
#define SHARED_POOL_SIZE 534552
#define MDL_LENGTH 0x82818

#define CMD_GET_MODULE 112
#define CMD_READ_PHYS 113
#define CMD_WRITE_PHYS 114
#define CMD_INIT 115
#define CMD_KBD_TICK 116

#define HOOK_CONTROL_CODE 2236419
#define HOOK_MAP_CODE 2236423

#define STATUS_NOT_READY 0xC0000001
#define STATUS_INVALID_PARAM 0xC000000D
#define STATUS_ACCESS_DENIED 0xC0000022
#define STATUS_TIMEOUT 0xC0000101

typedef struct _SHARED_HEADER
{
    UINT32 RequestIndex;
    UINT32 RequestPending;
    UINT32 ResponseIndex;
    UINT32 Reserved;
    UINT32 Ready;
} SHARED_HEADER, *PSHARED_HEADER;

typedef struct _REQUEST_ENTRY
{
    UINT64 Pad0[3];
    UINT64 PrevTag;
    UINT64 Checksum;
    UINT64 Pad1;
    UINT64 ProcessId;
    UINT64 Address;
    UINT64 Pad2;
    UINT64 Buffer;
    UINT64 Size;
    UINT64 Mode;
    UINT32 Param0;
    UINT32 Param1;
    UINT64 Data[509];
} REQUEST_ENTRY, *PREQUEST_ENTRY;

typedef struct _RESPONSE_ENTRY
{
    UINT64 PrevTag;
    UINT64 Tag;
    UINT64 Status;
    UINT64 Pad0;
    UINT64 DirBase;
    UINT64 ImageBase;
} RESPONSE_ENTRY, *PRESPONSE_ENTRY;

typedef struct _FAKE_DEVICE_OBJECT
{
    UINT32 Type;
    UCHAR Padding[220];
    UINT64 OriginalMajorFunction;
} FAKE_DEVICE_OBJECT;

extern PVOID P;
extern PMDL MemoryDescriptorList;
extern BOOLEAN Initialized;
extern UINT64 MappedUserAddress;
extern ULONG EprocessDirBaseOffset;
extern PVOID NullDriverObject;
extern PVOID NullDeviceObject;
extern PVOID OriginalMajorFunction;
extern UINT32 CpuFeatureFlags;

NTSTATUS MapSharedRegion(VOID);
NTSTATUS ProcessRequests(VOID);
UINT64 GetTargetProcessId(VOID);
UINT64 TranslateVirtualToPhysical(UINT64 DirectoryBase, UINT64 VirtualAddress);
NTSTATUS PhysicalMemoryOperation(UINT64 DirectoryBase, UINT64 TargetAddress, UINT64 Dst, UINT64 Src, SIZE_T Size, BOOLEAN Write);
NTSTATUS SaveMappedAddress(UINT64 Address);
NTSTATUS SetupNullDriverHook(VOID);
VOID CpuFeatureDetect(VOID);
UINT64 ReadCr3(VOID);
ULONG FindDirectoryBaseOffset(VOID);
char HookedDispatch(UINT64 Param0, UINT64 Param1, UINT64 Param2, UINT64 Param3, UINT64 Param4, int Unknown0, int Command, UINT64 OutStatus);
