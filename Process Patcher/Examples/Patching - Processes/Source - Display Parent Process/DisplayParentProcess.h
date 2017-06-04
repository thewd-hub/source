///////////////////////////////////////////////////
// Displays information about the parent process //
///////////////////////////////////////////////////

// unreferenced formal parameter
#pragma warning(disable:4100)

#define TH32CS_SNAPPROCESS	0x00000002

typedef struct tagPROCESSENTRY32
{
	DWORD dwSize;
	DWORD cntUsage;
	DWORD th32ProcessID;
	ULONG_PTR th32DefaultHeapID;
	DWORD th32ModuleID;
	DWORD cntThreads;
	DWORD th32ParentProcessID;
	LONG pcPriClassBase;
	DWORD dwFlags;
	CHAR szExeFile[MAX_PATH];
} PROCESSENTRY32;

typedef LONG NTSTATUS;
typedef struct _PEB* PPEB;
typedef LONG KPRIORITY;

typedef enum _PROCESS_INFORMATION_CLASS
{
	ProcessBasicInformation
} PROCESS_INFORMATION_CLASS;

typedef struct _PROCESS_BASIC_INFORMATION
{
	NTSTATUS ExitStatus;
	PPEB PebBaseAddress;
	ULONG_PTR AffinityMask;
	KPRIORITY BasePriority;
	ULONG_PTR UniqueProcessId;
	ULONG_PTR InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION;

typedef NTSTATUS (NTAPI* NTQUERYINFORMATIONPROCESS)(HANDLE hProcess, PROCESS_INFORMATION_CLASS pProcessInformationClass, PVOID pInformation, ULONG pInformationLength, PULONG pReturnLength);

typedef DWORD (WINAPI* GETMODULEBASENAME)(HANDLE hProcess, HMODULE hModule, LPTSTR lpBaseName, DWORD nSize);

typedef HANDLE (WINAPI* CREATETOOLHELP32SNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID);
typedef BOOL (WINAPI* PROCESS32FIRST)(HANDLE hSnapshot, PROCESSENTRY32* lppe);
typedef BOOL (WINAPI* PROCESS32NEXT)(HANDLE hSnapshot, PROCESSENTRY32* lppe);