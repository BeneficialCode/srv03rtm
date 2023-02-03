/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    pnpi.h

Abstract:

    This module contains the internal structure definitions and APIs used by
    the kernel-mode Plug and Play manager.

Author:

    Lonny McMichael (lonnym) 02/08/1995


Revision History:


--*/

#ifndef _KERNEL_PNPI_
#define _KERNEL_PNPI_

#include <wdmguid.h>
#include "regstrp.h"

#define MIN_CONFLICT_LIST_SIZE  (sizeof(PLUGPLAY_CONTROL_CONFLICT_LIST) - sizeof(PLUGPLAY_CONTROL_CONFLICT_ENTRY) + sizeof(PLUGPLAY_CONTROL_CONFLICT_STRINGS))

typedef struct _DEVICE_NODE DEVICE_NODE, *PDEVICE_NODE;
//
// Extract DeviceNode from DeviceObject.
//
#define PP_DO_TO_DN(DO)                \
    ((PDEVICE_NODE)((DO)? (DO)->DeviceObjectExtension->DeviceNode : NULL))
//
// Macros to save useful information into memory dumps.
//

#define PP_SAVE_DEVNODE_TO_TRIAGE_DUMP(dn) {                                                                        \
    if((dn)) {                                                                                                      \
        IoAddTriageDumpDataBlock(dn, sizeof(DEVICE_NODE));                                                          \
        if ((dn)->InstancePath.Length != 0) {                                                                       \
            IoAddTriageDumpDataBlock(&(dn)->InstancePath.Length, sizeof((dn)->InstancePath.Length));                \
            IoAddTriageDumpDataBlock((dn)->InstancePath.Buffer, (dn)->InstancePath.Length);                         \
        }                                                                                                           \
        if ((dn)->ServiceName.Length != 0) {                                                                        \
            IoAddTriageDumpDataBlock(&(dn)->ServiceName.Length, sizeof((dn)->ServiceName.Length));                  \
            IoAddTriageDumpDataBlock((dn)->ServiceName.Buffer, (dn)->ServiceName.Length);                           \
        }                                                                                                           \
        if ((dn)->Parent && (dn)->Parent->ServiceName.Length != 0) {                                                \
            IoAddTriageDumpDataBlock(&(dn)->Parent->ServiceName.Length, sizeof((dn)->Parent->ServiceName.Length));  \
            IoAddTriageDumpDataBlock((dn)->Parent->ServiceName.Buffer, (dn)->Parent->ServiceName.Length);           \
        }                                                                                                           \
    }                                                                                                               \
}

#define PP_SAVE_DRIVEROBJECT_TO_TRIAGE_DUMP(drvo) {                                                                 \
    if(drvo) {                                                                                                      \
        IoAddTriageDumpDataBlock(drvo, (drvo)->Size);                                                               \
        if((drvo)->DriverName.Length != 0) {                                                                        \
            IoAddTriageDumpDataBlock(&(drvo)->DriverName.Length, sizeof((drvo)->DriverName.Length));                \
            IoAddTriageDumpDataBlock((drvo)->DriverName.Buffer, (drvo)->DriverName.Length);                         \
        }                                                                                                           \
    }                                                                                                               \
}

#define PP_SAVE_DEVICEOBJECT_TO_TRIAGE_DUMP(do) {                                                                   \
    if((do)) {                                                                                                      \
        IoAddTriageDumpDataBlock(do, (do)->Size);                                                                   \
        PP_SAVE_DRIVEROBJECT_TO_TRIAGE_DUMP((do)->DriverObject);                                                    \
        PP_SAVE_DEVNODE_TO_TRIAGE_DUMP(PP_DO_TO_DN(do));                                                            \
    }                                                                                                               \
}    

#define GUID_STRING_LEN         39
#define MAX_DEVICE_ID_LEN       200     // size in chars
#define MAX_SERVICE_NAME_LEN    256     // in characters
//
// PNP_EVENT_LIST
//
//  This is the head of the master device event list for both user-mode and
//  kernel-mode.
//

typedef struct _PNP_DEVICE_EVENT_LIST {
    NTSTATUS    Status;
    KMUTEX      EventQueueMutex;
    KGUARDED_MUTEX Lock;
    LIST_ENTRY  List;
} PNP_DEVICE_EVENT_LIST, *PPNP_DEVICE_EVENT_LIST;

//
// PNP_DEVICE_EVENT_ENTRY
//
// One of these structures is allocated for each dynamic device event and
// is removed after the event has been posted to all waiting recipients.
// The notify block contains a pointer to this list.
//

typedef struct _PNP_DEVICE_EVENT_ENTRY {
    LIST_ENTRY                          ListEntry;
    ULONG                               Argument;
    PKEVENT                             CallerEvent;
    PDEVICE_CHANGE_COMPLETE_CALLBACK    Callback;
    PVOID                               Context;
    PPNP_VETO_TYPE                      VetoType;
    PUNICODE_STRING                     VetoName;
    PLUGPLAY_EVENT_BLOCK                Data;
} PNP_DEVICE_EVENT_ENTRY, *PPNP_DEVICE_EVENT_ENTRY;

//
// Defines the enum type to distinguish between REMOVE device
// and EJECT device.
//

typedef enum _PLUGPLAY_DEVICE_DELETE_TYPE {
    QueryRemoveDevice,
    CancelRemoveDevice,
    RemoveDevice,
    SurpriseRemoveDevice,
    EjectDevice,
    RemoveFailedDevice,
    RemoveUnstartedFailedDevice,
    MaxDeviceDeleteType
} PLUGPLAY_DEVICE_DELETE_TYPE, *PPLUGPLAY_DEVICE_DELETE_TYPE;


//++
//
// VOID
// PiWstrToUnicodeString(
//     OUT PUNICODE_STRING u,
//     IN  PCWSTR p
//     )
//
//--
#define PiWstrToUnicodeString(u, p) {                                       \
    if (p) {                                                                \
        (u)->Length = ((u)->MaximumLength = sizeof((p))) - sizeof(WCHAR);   \
    } else {                                                                \
        (u)->Length = (u)->MaximumLength = 0;                               \
    }                                                                       \
    (u)->Buffer = (p);                                                      \
}

//++
//
// VOID
// PiUlongToUnicodeString(
//     OUT    PUNICODE_STRING u,
//     IN OUT PWCHAR ub,
//     IN     ULONG ubl,
//     IN     ULONG i
//     )
//
//--
#define PiUlongToUnicodeString(u, ub, ubl, i)                                                                               \
    {                                                                                                                       \
        PWCHAR end;                                                                                                         \
        LONG len;                                                                                                           \
                                                                                                                            \
        StringCchPrintfExW((PWCHAR)(ub), (ubl) / sizeof(WCHAR), &end, NULL, 0, REGSTR_VALUE_STANDARD_ULONG_FORMAT, (i));    \
        len = (LONG)(end - (PWCHAR)(ub));                                                                                   \
        (u)->MaximumLength = (USHORT)(ubl);                                                                                 \
        (u)->Length = (len == -1) ? (USHORT)(ubl) : (USHORT)len * sizeof(WCHAR);                                            \
        (u)->Buffer = (PWSTR)(ub);                                                                                          \
    }

//++
//
// VOID
// PiUlongToInstanceKeyUnicodeString(
//     OUT    PUNICODE_STRING u,
//     IN OUT PWCHAR ub,
//     IN     ULONG ubl,
//     IN     ULONG i
//     )
//
//--
#define PiUlongToInstanceKeyUnicodeString(u, ub, ubl, i)                                                                \
    {                                                                                                                   \
        PWCHAR end;                                                                                                     \
        LONG len;                                                                                                       \
                                                                                                                        \
        StringCchPrintfExW((PWCHAR)(ub), (ubl) / sizeof(WCHAR), &end, NULL, 0, REGSTR_KEY_INSTANCE_KEY_FORMAT, (i));    \
        len = (LONG)(end - (PWCHAR)(ub));                                                                               \
        (u)->MaximumLength = (USHORT)(ubl);                                                                             \
        (u)->Length = (len == -1) ? (USHORT)(ubl) : (USHORT)len * sizeof(WCHAR);                                        \
        (u)->Buffer = (PWSTR)(ub);                                                                                      \
    }

//
// The following macros convert between a Count of Wide Characters (CWC) and a Count
// of Bytes (CB).
//
#define CWC_TO_CB(c)    ((c) * sizeof(WCHAR))
#define CB_TO_CWC(c)    ((c) / sizeof(WCHAR))

//
// Macro to determine the number of elements in a statically
// initialized array.
//
#define ELEMENT_COUNT(x) (sizeof(x)/sizeof((x)[0]))

//
// Enter critical section and acquire a lock on the registry.  Both these
// mechanisms are required to prevent deadlock in the case where an APC
// routine calls this routine after the registry resource has been claimed
// in this case it would wait blocking this thread so the registry would
// never be released -> deadlock.  Critical sectioning the registry manipulation
// portion solves this problem
//
#define PiLockPnpRegistry(Exclusive) {  \
    KeEnterCriticalRegion();            \
    if (Exclusive) {                    \
        ExAcquireResourceExclusiveLite(     \
            &PpRegistryDeviceResource,  \
            TRUE);                      \
    } else {                            \
        ExAcquireResourceSharedLite(        \
            &PpRegistryDeviceResource,  \
            TRUE);                      \
    }                                   \
}

//
// Unblock write access to Pnp portion of registry.
//
#define PiUnlockPnpRegistry() {                     \
    ExReleaseResourceLite(&PpRegistryDeviceResource);   \
    KeLeaveCriticalRegion();                        \
}

#define PiIsPnpRegistryLocked(Exclusive)    \
    ((Exclusive) ? ExIsResourceAcquiredExclusiveLite(&PpRegistryDeviceResource) : \
                    ((ExIsResourceAcquiredSharedLite(&PpRegistryDeviceResource) > 0) ? TRUE : FALSE))

//
// Function to complete an event asynchronously.
//
VOID
PpCompleteDeviceEvent(
    IN OUT PPNP_DEVICE_EVENT_ENTRY  DeviceEvent,
    IN     NTSTATUS                 FinalStatus
    );

//
// Global PnP Manager initialization data.
//

extern PVOID PiScratchBuffer;

//
// Private Entry Points
//
BOOLEAN
PiRegSzToString(
    IN  PWCHAR RegSzData,
    IN  ULONG  RegSzLength,
    OUT PULONG StringLength  OPTIONAL,
    OUT PWSTR  *CopiedString OPTIONAL
    );

VOID
PiUserResponse(
    IN ULONG            Response,
    IN PNP_VETO_TYPE    VetoType,
    IN LPWSTR           VetoName,
    IN ULONG            VetoNameLength
    );

NTSTATUS
PiDeviceRegistration(
    IN PUNICODE_STRING DeviceInstancePath,
    IN BOOLEAN Add,
    IN PUNICODE_STRING ServiceKeyName OPTIONAL
    );

BOOLEAN
PiCompareGuid(
    CONST GUID *Guid1,
    CONST GUID *Guid2
    );

NTSTATUS
PiGetDeviceRegistryProperty(
    IN      PDEVICE_OBJECT   DeviceObject,
    IN      ULONG            ValueType,
    IN      PWSTR            ValueName,
    IN      PWSTR            KeyName,
    OUT     PVOID            Buffer,
    IN OUT  PULONG           BufferLength
    );

VOID
PpInitializeDeviceReferenceTable(
    VOID
    );

PVOID
NTAPI
PiAllocateGenericTableEntry (
    PRTL_GENERIC_TABLE Table,
    CLONG ByteSize
    );

VOID
NTAPI
PiFreeGenericTableEntry (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer
    );

VOID
PpRemoveDeviceActionRequests(
    IN PDEVICE_OBJECT DeviceObject
    );

typedef struct _SYSTEM_HIVE_LIMITS {
    ULONG Low;
    ULONG High;
} SYSTEM_HIVE_LIMITS, *PSYSTEM_HIVE_LIMITS;

VOID
PpSystemHiveLimitCallback(
    PSYSTEM_HIVE_LIMITS HiveLimits,
    ULONG Level
    );

extern SYSTEM_HIVE_LIMITS PpSystemHiveLimits;
extern BOOLEAN PpSystemHiveTooLarge;

extern BOOLEAN PpCallerInitializesRequestTable;

VOID
PpLogEvent(
    IN PUNICODE_STRING InsertionString1,
    IN PUNICODE_STRING InsertionString2,
    IN NTSTATUS Status,
    IN PVOID DumpData,
    IN ULONG DumpDataSize
    );

NTSTATUS
PpIrpQueryDeviceText(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_TEXT_TYPE DeviceTextType,
    IN LCID POINTER_ALIGNMENT LocaleId,
    OUT PWCHAR *Description
   );

#define PpQueryDeviceDescription(dn, desc)          PpIrpQueryDeviceText((dn)->PhysicalDeviceObject, DeviceTextDescription, PsDefaultSystemLocaleId, desc)
#define PpQueryDeviceLocationInformation(dn, loc)   PpIrpQueryDeviceText((dn)->PhysicalDeviceObject, DeviceTextLocationInformation, PsDefaultSystemLocaleId, loc)

NTSTATUS
PpIrpQueryCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PDEVICE_CAPABILITIES Capabilities
    );

NTSTATUS
PpIrpQueryResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST *Requirements
   );

NTSTATUS
PpIrpQueryID(
    IN PDEVICE_OBJECT DeviceObject,
    IN BUS_QUERY_ID_TYPE IDType,
    OUT PWCHAR *ID
    );

NTSTATUS
PpQueryID(
    IN PDEVICE_NODE DeviceNode,
    IN BUS_QUERY_ID_TYPE IDType,
    OUT PWCHAR *ID,
    OUT PULONG IDLength
    );

NTSTATUS
PpQueryDeviceID(
    IN PDEVICE_NODE DeviceNode,
    OUT PWCHAR *BusID,
    OUT PWCHAR *DeviceID
    );

#define PpQueryInstanceID(dn, id, l)    PpQueryID(dn, BusQueryInstanceID, id, l)
#define PpQueryHardwareIDs(dn, id, l)   PpQueryID(dn, BusQueryHardwareIDs, id, l)
#define PpQueryCompatibleIDs(dn, id, l) PpQueryID(dn, BusQueryCompatibleIDs, id, l)
#define PpQuerySerialNumber(dn, id)     PpIrpQueryID((dn)->PhysicalDeviceObject, BusQueryDeviceSerialNumber, id)

NTSTATUS
PpIrpQueryBusInformation(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PPNP_BUS_INFORMATION *BusInfo
    );

NTSTATUS
PpQueryBusInformation(
    IN PDEVICE_NODE DeviceNode
    );

NTSTATUS
PpSaveDeviceCapabilities (
    IN PDEVICE_NODE DeviceNode,
    IN PDEVICE_CAPABILITIES Capabilities
    );

NTSTATUS
PpBusTypeGuidInitialize(
    VOID
    );

USHORT
PpBusTypeGuidGetIndex(
    IN LPGUID BusTypeGuid
    );

NTSTATUS
PpBusTypeGuidGet(
    IN USHORT Index,
    IN OUT LPGUID BusTypeGuid
    );

extern BOOLEAN PpDisableFirmwareMapper;

#if defined(_X86_)

NTSTATUS
PnPBiosMapper(
    VOID
    );

NTSTATUS
PnPBiosGetBiosInfo(
    OUT PVOID *BiosInfo,
    OUT ULONG *BiosInfoLength
    );

VOID
PnPBiosShutdownSystem(
    IN ULONG Phase,
    IN OUT PVOID *Context
    );

NTSTATUS
PnPBiosInitializePnPBios(
    VOID
    );

#endif

//
// Firmware mapper external declarations.
//

BOOLEAN
PipIsFirmwareMapperDevicePresent(
    IN HANDLE KeyHandle
    );

VOID
MapperProcessFirmwareTree(
    IN BOOLEAN OnlyProcessSerialPorts
    );

VOID
MapperConstructRootEnumTree(
    IN BOOLEAN CreatePhantomDevices
    );

VOID
MapperFreeList(
    VOID
    );

VOID
MapperPhantomizeDetectedComPorts(
    VOID
    );

//
// True iff textmode setup.
//
extern BOOLEAN ExpInTextModeSetup;

VOID
PpMarkDeviceStackStartPending(
    IN PDEVICE_OBJECT   DeviceObject,
    IN BOOLEAN          Set
    );

NTSTATUS
PiControlMakeUserModeCallersCopy(
    PVOID           *Destination,
    PVOID           Src,
    ULONG           Length,
    ULONG           Alignment,
    KPROCESSOR_MODE CallerMode,
    BOOLEAN         AllocateDestination
    );

#if DBG

LONG
PiControlExceptionFilter(
    IN  PEXCEPTION_POINTERS ExceptionPointers
    );

#else

#define PiControlExceptionFilter(a)  EXCEPTION_EXECUTE_HANDLER

#endif


#endif // _KERNEL_PNPI_
