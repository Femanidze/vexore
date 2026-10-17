#include "header.h"

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    NTSTATUS Status;
    PVOID Pool;

    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    EprocessDirBaseOffset = FindDirectoryBaseOffset();

    Pool = ExAllocatePool2(POOL_FLAG_NON_PAGED, SHARED_POOL_SIZE, POOL_TAG);
    P = Pool;

    if (Pool == nullptr)
        return 0xC000009A;

    RtlZeroMemory(Pool, SHARED_POOL_SIZE);

    PSHARED_HEADER Header = (PSHARED_HEADER)Pool;
    Header->RequestIndex = 0;
    Header->RequestPending = 0;
    Header->ResponseIndex = 0;
    Header->Reserved = 0;
    Header->Ready = 0;

    Status = SetupNullDriverHook();
    if (Status >= 0)
        return 0;

    ExFreePoolWithTag(P, 0);
    return Status;
}
