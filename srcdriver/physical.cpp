#include "header.h"

UINT64 TranslateVirtualToPhysical(UINT64 DirectoryBase, UINT64 VirtualAddress)
{
    UINT64 Pml4Entry = 0;
    UINT64 PdptEntry = 0;
    UINT64 PdEntry = 0;
    UINT64 PtEntry = 0;
    SIZE_T Copied = 0;

    if (MmCopyMemory(&Pml4Entry, (PVOID)((DirectoryBase & 0xFFFFFFFFFF000ull) + 8 * ((VirtualAddress >> 39) & 0x1FF)), sizeof(UINT64), MM_COPY_ADDRESS_PHYSICAL, &Copied) < 0)
        return 0;
    if (!(Pml4Entry & 1))
        return 0;

    if (MmCopyMemory(&PdptEntry, (PVOID)((Pml4Entry & 0xFFFFFFFFFF000ull) + 8 * ((VirtualAddress >> 30) & 0x1FF)), sizeof(UINT64), MM_COPY_ADDRESS_PHYSICAL, &Copied) < 0)
        return 0;
    if (!(PdptEntry & 1))
        return 0;

    if (PdptEntry & 0x80)
        return (VirtualAddress & 0x3FFFFFFF) + (PdptEntry & 0xFFFFC00000000ull);

    if (MmCopyMemory(&PdEntry, (PVOID)((PdptEntry & 0xFFFFFFFFFF000ull) + 8 * ((VirtualAddress >> 21) & 0x1FF)), sizeof(UINT64), MM_COPY_ADDRESS_PHYSICAL, &Copied) < 0)
        return 0;
    if (!(PdEntry & 1))
        return 0;

    if (PdEntry & 0x80)
        return (VirtualAddress & 0x1FFFFF) + (PdEntry & 0xFFFFFFFE00000ull);

    if (MmCopyMemory(&PtEntry, (PVOID)((PdEntry & 0xFFFFFFFFFF000ull) + 8 * ((VirtualAddress >> 12) & 0x1FF)), sizeof(UINT64), MM_COPY_ADDRESS_PHYSICAL, &Copied) < 0)
        return 0;
    if (!(PtEntry & 1))
        return 0;

    return (VirtualAddress & 0xFFF) + (PtEntry & 0xFFFFFFFFFF000ull);
}

NTSTATUS PhysicalMemoryOperation(UINT64 DirectoryBase, UINT64 TargetAddress, UINT64 Dst, UINT64 Src, SIZE_T Size, BOOLEAN Write)
{
    UINT64 Remaining = Size;
    UINT64 CurrentVa = TargetAddress;
    UINT64 CurrentDst = Dst;
    UINT64 CurrentSrc = Src;

    if (DirectoryBase == 0 || TargetAddress == 0 || Dst == 0 || Size == 0)
        return STATUS_INVALID_PARAM;

    while (Remaining)
    {
        UINT64 Physical = TranslateVirtualToPhysical(DirectoryBase, CurrentVa);
        if (Physical == 0)
            break;

        UINT64 PageOffset = Physical & 0xFFF;
        SIZE_T Chunk = Remaining;
        if (PAGE_SIZE - PageOffset <= Remaining)
            Chunk = PAGE_SIZE - PageOffset;

        if (Write)
        {
            PVOID Mapped = MmMapIoSpace(*(PPHYSICAL_ADDRESS)&Physical, Chunk, MmCached);
            if (Mapped == nullptr)
                return STATUS_ACCESS_DENIED;
            RtlCopyMemory(Mapped, (PVOID)CurrentSrc, Chunk);
            MmUnmapIoSpace(Mapped, Chunk);
        }
        else
        {
            SIZE_T Copied = 0;
            if (MmCopyMemory((PVOID)CurrentDst, (PVOID)Physical, Chunk, MM_COPY_ADDRESS_PHYSICAL, &Copied) < 0 || Copied != Chunk)
                return STATUS_NOT_READY;
        }

        CurrentVa += Chunk;
        CurrentDst += Chunk;
        CurrentSrc += Chunk;
        Remaining -= Chunk;
        if (Remaining == 0)
            return 0;
    }
    return STATUS_TIMEOUT;
}
