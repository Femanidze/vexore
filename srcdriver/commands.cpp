#include "header.h"

NTSTATUS MapSharedRegion(VOID)
{
    PEPROCESS Process = nullptr;
    KAPC_STATE ApcState;
    PVOID Mapped = nullptr;
    UINT64 TargetPid = GetTargetProcessId();
    BOOLEAN Remap = FALSE;

    if (Initialized)
    {
        if (TargetPid != 0 && TargetPid != MappedUserAddress)
        {
            if (MemoryDescriptorList)
            {
                IoFreeMdl(MemoryDescriptorList);
                MemoryDescriptorList = nullptr;
            }
            Initialized = FALSE;
            MappedUserAddress = 0;
            ((PSHARED_HEADER)P)->Ready = 0;
            Remap = TRUE;
        }
        else
        {
            return 0;
        }
    }

    if (TargetPid == 0)
        return STATUS_NOT_READY;

    if (PsLookupProcessByProcessId((HANDLE)TargetPid, &Process) < 0)
        return STATUS_NOT_READY;

    PMDL Mdl = IoAllocateMdl(P, MDL_LENGTH, FALSE, FALSE, nullptr);
    MemoryDescriptorList = Mdl;
    if (Mdl == nullptr)
    {
        ObfDereferenceObject(Process);
        return STATUS_NOT_READY;
    }

    MmBuildMdlForNonPagedPool(Mdl);

    KeStackAttachProcess(Process, &ApcState);
    Mapped = MmMapLockedPagesSpecifyCache(Mdl, UserMode, MmCached, nullptr, FALSE, NormalPagePriority | MdlMappingNoExecute);
    KeUnstackDetachProcess(&ApcState);

    if (Mapped == nullptr)
    {
        ObfDereferenceObject(Process);
        return STATUS_NOT_READY;
    }

    PSHARED_HEADER Header = (PSHARED_HEADER)P;
    Header->RequestIndex = 0;
    Header->RequestPending = 0;
    Header->ResponseIndex = 0;
    Header->Reserved = 0;
    Header->Ready = 1;

    MappedUserAddress = (UINT64)Mapped;
    SaveMappedAddress((UINT64)Mapped);
    Initialized = TRUE;

    ObfDereferenceObject(Process);
    (void)Remap;
    return 0;
}

NTSTATUS ProcessRequests(VOID)
{
    if (Initialized == FALSE || P == nullptr || ((PSHARED_HEADER)P)->Ready == 0)
        return STATUS_NOT_READY;

    PSHARED_HEADER Header = (PSHARED_HEADER)P;

    while (Header->RequestPending != Header->RequestIndex)
    {
        UINT32 Index = Header->RequestPending;
        PREQUEST_ENTRY Req = (PREQUEST_ENTRY)((UINT64)P + REQUEST_ENTRY_SIZE * (Index & 0x3F) + 32);
        PRESPONSE_ENTRY Resp = (PRESPONSE_ENTRY)((UINT64)P + RESPONSE_AREA_OFFSET + (UINT64)Index * sizeof(RESPONSE_ENTRY));

        UINT64 Checksum = Req->Checksum ^ XOR_KEY;
        if (Checksum == 0)
        {
            Header->ResponseIndex = (UINT32)((UINT64)Index + 1);
            Header->RequestPending = (UINT32)((UINT64)Index + 1);
            continue;
        }

        Resp->Tag = Req->Checksum;

        if (Checksum == CMD_INIT)
        {
            UINT64 Pid = Req->Buffer ^ XOR_KEY;
            PEPROCESS Process = nullptr;

            if (PsLookupProcessByProcessId((HANDLE)Pid, &Process) < 0)
            {
                Resp->Status = 0x21524110D337C2FBull;
            }
            else
            {
                Resp->DirBase = *(UINT64*)((UINT64)Process + EprocessDirBaseOffset) ^ XOR_KEY;
                Resp->ImageBase = (UINT64)PsGetProcessSectionBaseAddress(Process) ^ XOR_KEY;
                Resp->Status = XOR_KEY;
                ObfDereferenceObject(Process);
            }
        }
        else if (Checksum == CMD_READ_PHYS || Checksum == CMD_WRITE_PHYS)
        {
            UINT64 DirectoryBase = Req->ProcessId ^ XOR_KEY;
            UINT64 TargetAddress = Req->Address ^ XOR_KEY;
            UINT64 UserBuffer = Req->Buffer ^ XOR_KEY;
            UINT64 Size = Req->Size ^ XOR_KEY;

            if (Size > PAGE_SIZE)
                Size = PAGE_SIZE;

            UINT64 Dst = UserBuffer;
            UINT64 Src = (UINT64)&Req->Data;

            if (Checksum == CMD_WRITE_PHYS)
            {
                Dst = TargetAddress;
                Src = (UINT64)&Req->Data;
            }

            Resp->Status = (UINT64)PhysicalMemoryOperation(DirectoryBase, TargetAddress, Dst, Src, Size, Checksum == CMD_WRITE_PHYS) ^ XOR_KEY;
        }
        else if (Checksum == CMD_KBD_TICK && (Req->Mode ^ XOR_KEY) == 2)
        {
            KIRQL OldIrql;
            ULONG Pending = 0;
            UCHAR Reserved[4] = { 0 };
            UINT64 CallbackContext[2] = { 0 };
            PDRIVER_OBJECT KbdDriver = nullptr;
            UNICODE_STRING DriverName;

            RtlInitUnicodeString(&DriverName, L"\\Driver\\kbdhid");
            if (ObReferenceObjectByName(&DriverName, OBJ_CASE_INSENSITIVE, nullptr, 0, *IoDriverObjectType, KernelMode, nullptr, (PVOID*)&KbdDriver) >= 0)
            {
                PLIST_ENTRY Entry = *(PLIST_ENTRY*)((UINT64*)KbdDriver)[1];
                while (Entry)
                {
                    UINT64 DeviceObject = ((UINT64*)Entry)[8];
                    if (DeviceObject)
                    {
                        for (int Index = 0; Index < 100; ++Index)
                        {
                            PUINT16 Fn = (PUINT16)((UINT64*)DeviceObject)[Index];
                            if (MmIsAddressValid(Fn) && *Fn == 3)
                            {
                                PVOID Callback = ((PVOID*)DeviceObject)[Index + 1];
                                if (MmIsAddressValid(Callback) && (UINT64)Callback > 0xFFFFF00000000000ull)
                                {
                                    Pending = 0;
                                    OldIrql = KeRaiseIrqlToDpcLevel();
                                    ((int(*)(UINT64, UINT64, UCHAR*, ULONG*))((UINT64)Fn))(DeviceObject, CallbackContext, Reserved, &Pending);
                                    KeLowerIrql(OldIrql);
                                    goto Done;
                                }
                            }
                        }
                    }
                    Entry = (PLIST_ENTRY)Entry->Flink;
                }
            Done:
                ObfDereferenceObject(KbdDriver);
            }

            Resp->Status = XOR_KEY;
        }
        else
        {
            Resp->Status = 0x21524110D337C2FBull;
        }

        Header->ResponseIndex = (UINT32)((UINT64)Index + 1);
        Header->RequestPending = (UINT32)((UINT64)Index + 1);
    }
    return 0;
}

char HookedDispatch(UINT64 Param0, UINT64 Param1, UINT64 Param2, UINT64 Param3, UINT64 Param4, int Unknown0, int Command, UINT64 OutStatus)
{
    int Status = 0;

    UNREFERENCED_PARAMETER(Param1);
    UNREFERENCED_PARAMETER(Param2);
    UNREFERENCED_PARAMETER(Param3);
    UNREFERENCED_PARAMETER(Param4);
    UNREFERENCED_PARAMETER(Unknown0);

    if (Command == HOOK_CONTROL_CODE)
    {
        Status = (int)ProcessRequests();
        *(int*)OutStatus = Status;
        *(UINT64*)(OutStatus + 8) = 0;
        return 1;
    }

    if (Command == HOOK_MAP_CODE)
    {
        Status = (int)MapSharedRegion();
        *(int*)OutStatus = Status;
        *(UINT64*)(OutStatus + 8) = 0;
        return 1;
    }

    if (OriginalMajorFunction)
        return (char)((int(*)(UINT64))OriginalMajorFunction)(Param0);

    return 0;
}
