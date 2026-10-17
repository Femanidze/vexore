#include "header.h"

FAKE_DEVICE_OBJECT SharedMajorFunctionArea;

extern "C"
{
    PVOID P = nullptr;
    PMDL MemoryDescriptorList = nullptr;
    BOOLEAN Initialized = FALSE;
    UINT64 MappedUserAddress = 0;
    ULONG EprocessDirBaseOffset = 0x28;
    PVOID NullDriverObject = nullptr;
    PVOID NullDeviceObject = nullptr;
    PVOID OriginalMajorFunction = nullptr;
    UINT32 CpuFeatureFlags = 0;

    UINT64 ReadCr3(VOID)
    {
        return __readcr3();
    }

    ULONG FindDirectoryBaseOffset(VOID)
    {
        ULONG Offset = 0;
        UINT64 Cr3;

        if (PsInitialSystemProcess == nullptr)
            return 0;

        Cr3 = __readcr3() & 0xFFFFFFFFFF000ull;
        for (UINT64 Offset64 = 0; Offset64 < PAGE_SIZE; Offset64 += sizeof(UINT64))
        {
            UINT64 Value = *(UINT64*)((UINT64)PsInitialSystemProcess + Offset64);
            if ((Value & 0xFFFFFFFFFF000ull) == Cr3 && Value != 0)
                return (ULONG)Offset64;
        }
        return 0;
    }

    UINT64 GetTargetProcessId(VOID)
    {
        UINT64 Target = 0;
        UNICODE_STRING KeyPath;
        UNICODE_STRING ValueName;
        OBJECT_ATTRIBUTES Attributes;
        HANDLE KeyHandle = nullptr;
        PKEY_VALUE_PARTIAL_INFORMATION Info = nullptr;
        ULONG ResultLength = 0;

        RtlInitUnicodeString(&KeyPath, L"\\Registry\\Machine\\Hardware\\Description\\System\\CentralProcessor\\0");
        RtlInitUnicodeString(&ValueName, L"FeatureSetMask");

        InitializeObjectAttributes(&Attributes, &KeyPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);

        if (ZwOpenKey(&KeyHandle, KEY_READ, &Attributes) < 0)
            return 0;

        Info = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool2(POOL_FLAG_NON_PAGED, 100, POOL_TAG);
        if (Info)
        {
            if (ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation, Info, 100, &ResultLength) >= 0)
                Target = *(UINT64*)&Info->Data[0] ^ XOR_KEY;
            ExFreePoolWithTag(Info, 0);
        }
        ZwClose(KeyHandle);
        return Target;
    }

    NTSTATUS SaveMappedAddress(UINT64 Address)
    {
        NTSTATUS Status;
        UNICODE_STRING KeyPath;
        UNICODE_STRING ValueName;
        OBJECT_ATTRIBUTES Attributes;
        HANDLE KeyHandle = nullptr;
        UINT64 Data = Address ^ XOR_KEY;

        RtlInitUnicodeString(&KeyPath, L"\\Registry\\Machine\\Hardware\\Description\\System\\CentralProcessor\\0");
        RtlInitUnicodeString(&ValueName, L"FeatureSetMask");

        InitializeObjectAttributes(&Attributes, &KeyPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);

        Status = ZwOpenKey(&KeyHandle, KEY_SET_VALUE, &Attributes);
        if (Status >= 0)
        {
            Status = ZwSetValueKey(KeyHandle, &ValueName, 0, REG_BINARY, &Data, sizeof(UINT64));
            ZwClose(KeyHandle);
        }
        return Status;
    }

    VOID CpuFeatureDetect(VOID)
    {
        int Info[4];
        UINT8 Flags = 0;

        __cpuid(Info, 0);
        int MaxLeaf = Info[0];

        __cpuid(Info, 1);
        if (Info[2] & (1 << 20))
        {
            Flags |= 8;
            if ((Info[2] & (1 << 27)) && (Info[2] & (1 << 28)))
            {
                UINT64 Xcr0 = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
                Flags = 8;
                if ((Xcr0 & 6) == 6)
                    Flags = 12;
            }
        }

        if (MaxLeaf >= 7)
        {
            __cpuidex(Info, 7, 0);
            UINT8 NewFlags = Flags | 2;
            if (!(Info[1] & (1 << 9)))
                NewFlags = Flags;
            Flags = NewFlags;
            if ((Info[1] & (1 << 5)) && (NewFlags & 4))
                Flags = NewFlags | 0x10;
        }

        CpuFeatureFlags = Flags | 1;
    }

    NTSTATUS SetupNullDriverHook(VOID)
    {
        NTSTATUS Status;
        UNICODE_STRING DriverName;
        PDRIVER_OBJECT DriverObject = nullptr;

        RtlInitUnicodeString(&DriverName, L"\\Driver\\Null");

        Status = ObReferenceObjectByName(&DriverName, OBJ_CASE_INSENSITIVE, nullptr, 0, *IoDriverObjectType, KernelMode, nullptr, (PVOID*)&DriverObject);
        if (Status < 0 || DriverObject == nullptr)
            return Status;

        NullDriverObject = DriverObject;
        NullDeviceObject = DriverObject->DeviceObject;

        if (NullDeviceObject == nullptr)
        {
            RtlZeroMemory(&SharedMajorFunctionArea, sizeof(FAKE_DEVICE_OBJECT));
            CpuFeatureFlags = 224;
        }
        else
        {
            RtlCopyMemory(&SharedMajorFunctionArea, NullDeviceObject, 224);
        }

        OriginalMajorFunction = SharedMajorFunctionArea.OriginalMajorFunction;
        SharedMajorFunctionArea.OriginalMajorFunction = (UINT64)HookedDispatch;
        DriverObject->DeviceObject = (PDEVICE_OBJECT)&SharedMajorFunctionArea;

        return 0;
    }
}
