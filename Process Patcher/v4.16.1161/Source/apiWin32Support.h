//////////////////////////////////////////////////////////
// Support functions and structures for accessing Win32 //
// functions (whether they are supported APIs or not)   //
//////////////////////////////////////////////////////////

#if (!defined(MAX_MODULE_NAME32))
#define MAX_MODULE_NAME32		255
#endif

#if (!defined(TH32CS_SNAPPROCESS))
#define TH32CS_SNAPPROCESS		0x00000002
#endif
#if (!defined(TH32CS_SNAPMODULE))
#define TH32CS_SNAPMODULE		0x00000008
#endif

#if (!defined(SE_DEBUG_PRIVILEGE))
#define SE_DEBUG_PRIVILEGE		20
#endif

#if (!defined(OBJ_INHERIT))
#define OBJ_INHERIT				0x00000002
#endif

#if (!defined(CLIENT_ID))
typedef struct _CLIENT_ID
{
	DWORD UniqueProcess;
	DWORD UniqueThread;
} CLIENT_ID;
#endif

#if (!defined(UNICODE_STRING))
typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING;
#endif

#if (!defined(NTSTATUS))
typedef LONG NTSTATUS;
#endif

#if (!defined(STATUS_SUCCESS))
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#endif

#if (!defined(STATUS_UNSUCCESSFUL))
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)
#endif

#if (!defined(STATUS_NOT_IMPLEMENTED))
#define STATUS_NOT_IMPLEMENTED ((NTSTATUS)0xC0000002L)
#endif

#if (!defined(STATUS_INVALID_INFO_CLASS))
#define STATUS_INVALID_INFO_CLASS ((NTSTATUS)0xC0000003L)
#endif

#if (!defined(STATUS_INFO_LENGTH_MISMATCH))
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
#endif

#if (!defined(STATUS_ACCESS_DENIED))
#define STATUS_ACCESS_DENIED ((NTSTATUS)0xC0000022L)
#endif

#if (!defined(NT_SUCCESS))
#define NT_SUCCESS(status) ((NTSTATUS)(status) >= STATUS_SUCCESS)
#endif

// KERNEL32.DLL API Functions
class API_KERNEL32_MODULE
{
	public:
		// process database
		typedef struct _PDB_9x
		{
			union _U1
			{
				struct _Win95A
				{
					DWORD DatabaseType;
					DWORD ReferenceCount;
					DWORD Reserved1;
					DWORD WaitEventObject;
				} Win95A;
				struct _Win9x
				{
					WORD DatabaseType;
					WORD ReferenceCount;
					DWORD WaitEventObject;
					DWORD Reserved1;
					DWORD Reserved2;
				} Win9x;
			} U1;
			DWORD TerminationStatus;
			DWORD Reverved1;
			DWORD DefaultHeap;
			DWORD MemoryContext;
			DWORD Flags;
			DWORD DOSPSPAddress;
			WORD DOSPSPSelector;
			WORD ModuleTableArray;
			WORD ThreadCount;
			WORD ThreadCountNotTerminated;
			WORD FreeLibraryCount;
			WORD ThreadRing0Count;
			DWORD SharedHeapHandle;
			DWORD Win16TaskDatabase;
			DWORD MemoryMappedFiles;
			DWORD EnvironmentDatabase;
			DWORD HandleTable;
			DWORD ParentPDB;
			DWORD ModuleReferenceList;
			DWORD ThreadList;
			DWORD DebuggerContextBlock;
			DWORD LocalHeapFreeEntry;
			DWORD InitialRing0Id;
			DWORD CriticalSection;
			DWORD Reserved2;
			DWORD Reserved3;
			DWORD Reserved4;
			DWORD ConsoleObjectHandle;
			DWORD Low32TLS;
			DWORD High32TLS;
			DWORD ProcessDWORD;
			DWORD ProcessGroup;
			DWORD ModuleReferenceListEntry;
			DWORD TopExceptionFilter; // Windows 98+
			DWORD BasePriority;
			DWORD FirstProcessHeap;
			DWORD FirstMovableHandleTable;
			DWORD Reserved5;
			DWORD KernelConsoleObject;
			WORD ProcessEnvironment;
			WORD ErrorMode;
			DWORD FinishedLoadingObject;
			WORD UTState;
		} PDB_9x;

		// thread database
		typedef struct _TDB_9x
		{
			union _U1
			{
				struct _Win95A
				{
					DWORD DatabaseType;
					DWORD ReferenceCount;
					//DWORD PDB;
					//DWORD EventObjectHandle;
				} Win95A;
				struct _Win9x
				{
					WORD DatabaseType;
					WORD ReferenceCount;
					DWORD EventObjectHandle;
				} Win9x;
			} U1;
			DWORD ExceptionList;
			DWORD StackTop;
			DWORD StackLow;
			WORD Win16TaskDatabase;
			WORD StackSelector16;
			DWORD SelectorManagerList;
			DWORD UserPointer;
			DWORD TIB;
			WORD TIBFlags;
			WORD Win16MutexCount;
			DWORD DebugContext;
			DWORD CurrentPriority;
			DWORD MessageQueue;
			DWORD TLSArray;
			DWORD PDB;
			DWORD Flags;
			DWORD TerminationStatus;
			WORD TIBSelector;
			WORD EmulatorSelector;
			DWORD HandleCount;
			DWORD WaitNodeList;
			// ---------- Windows ME ----------
			DWORD Reserved1;
			DWORD ThreadTableRing0;
			DWORD TDBX;
			DWORD StackBase;
			DWORD TerminationStack;
			DWORD EmulatorData;
			DWORD GetLastErrorCode;
			DWORD DebuggerContextBlock;
			DWORD DebuggerThread;
			DWORD ThreadContext;
			DWORD ExceptionList16;
			DWORD ThunkConnect;
			DWORD NegativeStackBase;
			DWORD CurrentStackSelector;
			DWORD StackSelectorTable;
			DWORD ThunkStackSelector16; // ...
		} TDB_9x;

		// 0x128
		typedef struct _PROCESSENTRY32
		{
			DWORD dwSize; // 0x00
			DWORD cntUsage; // 0x04
			DWORD th32ProcessID; // 0x08
			ULONG_PTR th32DefaultHeapID; // 0x0C
			DWORD th32ModuleID; // 0x10
			DWORD cntThreads; // 0x14
			DWORD th32ParentProcessID; // 0x18
			LONG pcPriClassBase; // 0x1C
			DWORD dwFlags; // 0x20
			CHAR szExeFile[MAX_PATH]; // 0x24
		} PROCESSENTRY32;

		// 0x224
		typedef struct _MODULEENTRY32
		{
			DWORD dwSize; // 0x00
			DWORD th32ModuleID; // 0x04
			DWORD th32ProcessID; // 0x08
			DWORD GlblcntUsage; // 0x0C
			DWORD ProccntUsage; // 0x10
			BYTE* modBaseAddr; // 0x14
			DWORD modBaseSize; // 0x18
			HMODULE hModule; // 0x1C
			CHAR szModule[MAX_MODULE_NAME32 + 1]; // 0x20
			CHAR szExePath[MAX_PATH]; // 0x124
		} MODULEENTRY32;

		typedef HANDLE (WINAPI* CREATETOOLHELP32SNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID);
		typedef BOOL (WINAPI* PROCESS32FIRST)(HANDLE hSnapshot, PROCESSENTRY32* lppe);
		typedef BOOL (WINAPI* PROCESS32NEXT)(HANDLE hSnapshot, PROCESSENTRY32* lppe);
		typedef BOOL (WINAPI* MODULE32FIRST)(HANDLE hSnapshot, MODULEENTRY32* lpme);
		typedef BOOL (WINAPI* MODULE32NEXT)(HANDLE hSnapshot, MODULEENTRY32* lpme);
		typedef HANDLE (WINAPI* OPENTHREAD)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId);

	private:
		CREATETOOLHELP32SNAPSHOT CreateToolhelp32Snapshot;
		PROCESS32FIRST Process32First;
		PROCESS32NEXT Process32Next;
		MODULE32FIRST Module32First;
		MODULE32NEXT Module32Next;
		OPENTHREAD OpenThread;

	public:
		void setCreateToolhelp32Snapshot(void* farPtr) { CreateToolhelp32Snapshot = (CREATETOOLHELP32SNAPSHOT)farPtr; }
		void setProcess32First(void* farPtr) { Process32First = (PROCESS32FIRST)farPtr; }
		void setProcess32Next(void* farPtr) { Process32Next = (PROCESS32NEXT)farPtr; }
		void setModule32First(void* farPtr) { Module32First = (MODULE32FIRST)farPtr; }
		void setModule32Next(void* farPtr) { Module32Next = (MODULE32NEXT)farPtr; }
		void setOpenThread(void* farPtr) { OpenThread = (OPENTHREAD)farPtr; }

		CREATETOOLHELP32SNAPSHOT getCreateToolhelp32Snapshot(void) { return CreateToolhelp32Snapshot; }
		PROCESS32FIRST getProcess32First(void) { return Process32First; }
		PROCESS32NEXT getProcess32Next(void) { return Process32Next; }
		MODULE32FIRST getModule32First(void) { return  Module32First; }
		MODULE32NEXT getModule32Next(void) { return Module32Next; }
		OPENTHREAD getOpenThread(void) { return OpenThread; }
};

// NTDLL.DLL API Functions
class API_NTDLL_MODULE
{
	public:
		typedef LONG KPRIORITY;

		// 0x08
		typedef struct _LIST_ENTRY
		{
			struct _LIST_ENTRY* volatile Flink; // 0x00
			struct _LIST_ENTRY* volatile Blink; // 0x04
		} LIST_ENTRY;

		// 0x58
		typedef struct _LDR_DATA_TABLE_ENTRY
		{
			LIST_ENTRY InLoadOrderLinks; // 0x00
			LIST_ENTRY InMemoryOrderLinks; // 0x08
			LIST_ENTRY InInitializationOrderLinks; // 0x10
			PVOID DllBase; // 0x18
			PVOID EntryPoint; // 0x1C
			ULONG SizeOfImage; // 0x20
			UNICODE_STRING FullDllName; // 0x24
			UNICODE_STRING BaseDllName; // 0x2C
			ULONG Flags; // 0x34
			USHORT LoadCount; // 0x34
			USHORT TlsIndex; // 0x3A
			union
			{
				LIST_ENTRY HashLinks; // 0x3C
				struct S1_1
				{
					PVOID SectionPointer; // 0x3C
					ULONG CheckSum; // 0x40
				} S1_1;
			};
			union
			{
				ULONG TimeDateStamp; // 0x44
				// ---------- Windows 2000 SP4 ----------
				// ---------- Windows XP ----------
				PVOID LoadedImports; // 0x44
			};
			PVOID EntryPointActivationContext; // 0x48 (_ACTIVATION_CONTEXT*)
			// ---------- Windows Server 2003 ----------
			// ---------- Windows XP SP2 ---------------
			PVOID PatchInformation; // 0x4C
			// ---------- Windows Vista ----------
			PVOID ForwarderLinks; // 0x50
			PVOID ServiceTagLinks; // 0x54
			PVOID StaticLinks; // 0x58
		} LDR_DATA_TABLE_ENTRY;

		// 0x28
		typedef struct _PEB_LDR_DATA
		{
			ULONG Length; // 0x00
			BOOLEAN Initialized; // 0x04
			HANDLE SsHandle; // 0x08
			LIST_ENTRY InLoadOrderModuleList; // 0x0C
			LIST_ENTRY InMemoryOrderModuleList; // 0x14
			LIST_ENTRY InInitializationOrderModuleList; // 0x1C
			// ---------- Windows XP ----------
			// ---------- Windows Server 2003 ----------
			PVOID EntryInProgress; // 0x24
		} PEB_LDR_DATA;

		// 0x08
		typedef struct _PEB_FREE_BLOCK
		{
			struct _PEB_FREE_BLOCK* Next; // 0x00
			ULONG Size; // 0x04
		} PEB_FREE_BLOCK;

		// 0x238
		typedef struct _PEB
		{
			BOOLEAN InheritedAddressSpace; // 0x00
			BOOLEAN ReadImageFileExecOptions; // 0x01
			BOOLEAN BeingDebugged; // 0x02
			BOOLEAN SpareBool; // 0x03
			HANDLE Mutant; // 0x04
			PVOID ImageBaseAddress; // 0x08
			PEB_LDR_DATA* Ldr; // 0x0C
			struct _RTL_USER_PROCESS_PARAMETERS* ProcessParameters; // 0x10
			PVOID SubSystemData; // 0x14
			PVOID ProcessHeap; // 0x18
			PVOID FastPebLock; // 0x1C (RTL_CRITICAL_SECTION*)
			union
			{
				// ---------- Windows NT4 ----------
				// ---------- Windows 2000 ----------
				// ---------- Windows XP ----------
				struct S1_1
				{
					PVOID FastPebLockRoutine; // 0x20
					PVOID FastPebUnlockRoutine; // 0x24
				} S1_1;
				// ---------- Windows Server 2003 ----------
				struct S1_2
				{
					PVOID SparePtr1; // 0x20
					PVOID SparePtr2; // 0x24
				} S1_2;
				// ---------- Windows Server 2003 SP1 ----------
				struct S1_3
				{
					PVOID AtlThunkSListPtr; // 0x20
					PVOID SparePtr2; // 0x24
				} S1_3;
			};
			ULONG EnvironmentUpdateCount; // 0x28
			PVOID KernelCallbackTable; // 0x2C
			union
			{
				// ---------- Windows NT4 ----------
				struct S2_1
				{
					HANDLE EventLogSection; // 0x30
					PVOID EventLog; // 0x34
				} S2_1;
				// ---------- Windows 2000 ----------
				struct S2_2
				{
					ULONG SystemReserved[2];
				} S2_2;
				// ---------- Windows XP ----------
				// ---------- Windows Server 2003 ----------
				struct S2_3
				{
					ULONG SystemReserved[1]; // 0x30
					union
					{
						ULONG ExecuteOptions; // 0x34
						ULONG SpareBits; // 0x34
					};
				} S2_3;
				// ---------- Windows XP SP2 ----------
				struct S2_4
				{
					ULONG SystemReserved[1]; // 0x30
					PVOID AtlThunkSListPtr32; // 0x34
				} S2_4;
				// ---------- Windows Server 2003 SP1 ----------
				struct S2_5
				{
					ULONG SystemReserved[1]; // 0x30
					ULONG SpareUlong; // 0x34
				} S2_5;
			};
			PEB_FREE_BLOCK* FreeList; // 0x38
			ULONG TlsExpansionCounter; // 0x3C
			PVOID TlsBitmap; // 0x40
			ULONG TlsBitmapBits[2]; // 0x44
			PVOID ReadOnlySharedMemoryBase; // 0x4C
			PVOID ReadOnlySharedMemoryHeap; // 0x50
			PVOID* ReadOnlyStaticServerData; // 0x54
			PVOID AnsiCodePageData; // 0x58
			PVOID OemCodePageData; // 0x5C
			PVOID UnicodeCaseTableData; // 0x60
			ULONG NumberOfProcessors; // 0x64
			ULONG NtGlobalFlag; // 0x68
			LARGE_INTEGER CriticalSectionTimeout; // 0x70
			ULONG HeapSegmentReserve; // 0x78
			ULONG HeapSegmentCommit; // 0x7C
			ULONG HeapDeCommitTotalFreeThreshold; // 0x80
			ULONG HeapDeCommitFreeBlockThreshold; // 0x84
			ULONG NumberOfHeaps; // 0x88
			ULONG MaximumNumberOfHeaps; // 0x8C
			PVOID* ProcessHeaps; // 0x90
			PVOID GdiSharedHandleTable; // 0x94
			PVOID ProcessStarterHelper; // 0x98
			PVOID GdiDCAttributeList; // 0x9C
			PVOID LoaderLock; // 0xA0 (_RTL_CRITICAL_SECTION*)
			ULONG OSMajorVersion; // 0xA4
			ULONG OSMinorVersion; // 0xA8
			USHORT OSBuildNumber; // 0xAC
			USHORT OSCSDVersion; // 0xAE
			ULONG OSPlatformId; // 0xB0
			ULONG ImageSubsystem; // 0xB4
			ULONG ImageSubsystemMajorVersion; // 0xB8
			ULONG ImageSubsystemMinorVersion; // 0xBC
			ULONG ImageProcessAffinityMask; // 0xC0
			ULONG GdiHandleBuffer[34]; // 0xC4 (GDI_HANDLE_BUFFER_SIZE)
			// ---------- Windows 2000 ----------
			PVOID PostProcessInitRoutine; // 0x14C
			PVOID TlsExpansionBitmap; // 0x150
			UCHAR TlsExpansionBitmapBits[128]; // 0x154
			ULONG SessionId; // 0x1D4
			union
			{
				// ---------- Windows 2000 ----------
				struct S3_1
				{
					PVOID AppCompatInfo; // 0x1D8
					UNICODE_STRING CSDVersion; // 0x1DC
					ULONG Unused1; // 0x1E4
				} S3_1;
				// ---------- Windows XP ----------
				// ---------- Windows Server 2003 ----------
				// ---------- Windows Vista ----------
				struct S3_2
				{
					LARGE_INTEGER AppCompatFlags; // 0x1D8
					LARGE_INTEGER AppCompatFlagsUser; // 0x1E0
				} S3_2;
			};
			// ---------- Windows XP ----------
			PVOID pShimData; // 0x1E8
			PVOID AppCompatInfo; // 0x1EC
			UNICODE_STRING CSDVersion; // 0x1F0
			ULONG ActivationContextData; // 0x1F8 (_ACTIVATION_CONTEXT_DATA*)
			ULONG ProcessAssemblyStorageMap; // 0x1FC (_ASSEMBLY_STORAGE_MAP*)
			ULONG SystemDefaultActivationContextData; // 0x200 (_ACTIVATION_CONTEXT_DATA*)
			ULONG SystemAssemblyStorageMap; // 0x204 (_ASSEMBLY_STORAGE_MAP*)
			ULONG MinimumStackCommit; // 0x208
			// ---------- Windows Server 2003 ----------
			PVOID* FlsCallBack; // 0x20C
			LIST_ENTRY FlsListHead; // 0x210
			PVOID FlsBitmap; // 0x218
			ULONG FlsBitmapBits[4]; // 0x21C
			ULONG FlsHighIndex; // 0x22C
			// ---------- Windows Vista ----------
			PVOID WerRegistrationData; // 0x230
			PVOID WerShipAssertPtr; // 0x234
			// -----------------------------------
			PVOID pContextData;
			PVOID pImageHeaderHash;
		} PEB;

		// 0x1C
		typedef struct _NT_TIB
		{
			struct _EXCEPTION_REGISTRATION_RECORD* ExceptionList; // 0x00
			PVOID StackBase; // 0x04
			PVOID StackLimit; // 0x08
			PVOID SubSystemTib; // 0x0C
			union
			{
				PVOID FiberData; // 0x10
				ULONG Version; // 0x10
			};
			PVOID ArbitraryUserPointer; // 0x14
			struct _NT_TIB* Self; // 0x18
		} NT_TIB;

		// 0x4E0
		typedef struct _GDI_TEB_BATCH
		{
			ULONG Offset; // 0x00
			ULONG HDC; // 0x04
			ULONG Buffer[310]; // 0x08 (GDI_BATCH_BUFFER_SIZE)
		} GDI_TEB_BATCH;

		// 0xFBC
		typedef struct _TEB
		{
			NT_TIB NtTib; // 0x00
			PVOID EnvironmentPointer; // 0x1C
			CLIENT_ID ClientId; // 0x20
			PVOID ActiveRpcHandle; // 0x28
			PVOID ThreadLocalStoragePointer; // 0x2C
			PEB* ProcessEnvironmentBlock; // 0x30
			ULONG LastErrorValue; // 0x34
			ULONG CountOfOwnedCriticalSections; // 0x38
			PVOID CsrClientThread; // 0x3C
			PVOID Win32ThreadInfo; // 0x40
			union
			{
				// ---------- Windows NT4 ----------
				// ---------- Windows 2000 ----------
				struct S1_1
				{
					ULONG Win32ClientInfo[31]; // 0x44 (WIN32_CLIENT_INFO_LENGTH)
				} S1_1;
				// ---------- Windows XP ----------
				// ---------- Windows Server 2003 ----------
				struct S1_4
				{
					ULONG User32Reserved[26]; // 0x44
					ULONG UserReserved[5]; // 0xAC
				} S1_4;
			};
			PVOID WOW32Reserved; // 0xC0
			LCID CurrentLocale; // 0xC4
			ULONG FpSoftwareStatusRegister; // 0xC8
			PVOID SystemReserved1[54]; // 0xCC
			NTSTATUS ExceptionCode; // 0x1A4
			PVOID ActivationContextStackPointer[5]; // 0x1A8 (_ACTIVATION_CONTEXT_STACK*)
			UCHAR SpareBytes1[24]; // 0x1BC
			GDI_TEB_BATCH GdiTebBatch; // 0x1D4
			CLIENT_ID RealClientId; // 0x6B4
			HANDLE GdiCachedProcessHandle; // 0x6BC
			ULONG GdiClientPID; // 0x6C0
			ULONG GdiClientTID; // 0x6C4
			PVOID GdiThreadLocalInfo; // 0x6C8
			ULONG Win32ClientInfo[62]; // 0x6CC
			PVOID glDispatchTable[233]; // 0x7C4
			ULONG glReserved1[29]; // 0xB68
			PVOID glReserved2; // 0xBDC
			PVOID glSectionInfo; // 0xBE0
			PVOID glSection; // 0xBE4
			PVOID glTable; // 0xBE8
			PVOID glCurrentRC; // 0xBEC
			PVOID glContext; // 0xBF0
			ULONG LastStatusValue; // 0xBF4
			UNICODE_STRING StaticUnicodeString; // 0xBF8
			WCHAR StaticUnicodeBuffer[261]; // 0xC00 (STATIC_UNICODE_BUFFER_LENGTH)
			PVOID DeallocationStack; // 0xE0C
			PVOID TlsSlots[TLS_MINIMUM_AVAILABLE]; // 0xE0C
			LIST_ENTRY TlsLinks; // 0xF10
			PVOID Vdm; // 0xF18
			PVOID ReservedForNtRpc; // 0xF1C
			PVOID DbgSsReserved[2]; // 0xF20
			ULONG HardErrorsMode; // 0xF28
			union
			{
				// ---------- Windows XP ----------
				// ---------- Windows Server 2003 ----------
				struct S2_1
				{
					PVOID Instrumentation[16]; // 0xF2C
				} S2_1;
				// ---------- Windows Server 2003 SP1 ----------
				struct S2_2
				{
					PVOID Instrumentation[15]; // 0xF2C
					PVOID EtwTraceData; // 0xF68
				} S2_2;
			};
			PVOID WinSockData; // 0xF6C
			ULONG GdiBatchCount; // 0xF70
			UCHAR InDbgPrint; // 0xF74
			UCHAR FreeStackOnTermination; // 0xF75
			UCHAR HasFiberData; // 0xF76
			UCHAR IdealProcessor; // 0xF77
			union
			{
				// ---------- Windows XP ----------
				// ---------- Windows Server 2003 ----------
				struct S3_1
				{
					ULONG Spare3; // 0xF78
				} S3_1;
				// ---------- Windows Server 2003 SP1 ----------
				struct S3_2
				{
					ULONG GuaranteedStackBytes; // 0xF78
				} S3_2;
			};
			PVOID ReservedForPerf; // 0xF7C
			PVOID ReservedForOle; // 0xF80
			ULONG WaitingOnLoaderLock; // 0xF84
			// ---------- Windows 2000 ----------
			PVOID Wx86Thread[3]; // 0xF88 (_Wx86ThreadState)
			PVOID* TlsExpansionSlots; // 0xF94
			ULONG ImpersonationLocale; // 0xF98
			ULONG IsImpersonating; // 0xF9C
			PVOID NlsCache; // 0xFA0
			PVOID pShimData; // 0xFA4
			ULONG HeapVirtualAffinity; // 0xFA8
			PVOID CurrentTransactionHandle; // 0xFAC
			PVOID ActiveFrame; // 0xFB0 (_TEB_ACTIVE_FRAME*)
			union
			{
				// ---------- Windows Server 2003 ----------
				struct S4_1
				{
					PVOID FlsData; // 0xFB4
					// ---------- Windows Server 2003 SP1 ----------
					UCHAR SafeThunkCall; // 0xFB8
					UCHAR BooleanSpare[3]; // 0xFB9
				} S4_1;
				// ---------- Windows XP ----------
				struct S4_2
				{
					UCHAR SafeThunkCall; // 0xFB4
					UCHAR BooleanSpare[3]; // 0xFB5
					ULONG Unused1; // 0xFB8
				} S4_2;
			};
		} TEB;

		typedef enum _SYSTEM_INFORMATION_CLASS
		{
			SystemBasicInformation, // 0x00
			SystemProcessorInformation, // 0x01
			SystemPerformanceInformation, // 0x02
			SystemTimeOfDayInformation, // 0x03
			SystemPathInformation, // 0x04
			SystemProcessInformation, // 0x05
			SystemCallCountInformation, // 0x06
			SystemDeviceInformation, // 0x07
			SystemProcessorPerformanceInformation, // 0x08
			SystemFlagsInformation, // 0x09
			SystemCallTimeInformation, // 0x0A
			SystemModuleInformation, // 0x0B
			SystemLocksInformation, // 0x0C
			SystemStackTraceInformation, // 0x0D
			SystemPagedPoolInformation, // 0x0E
			SystemNonPagedPoolInformation, // 0x0F
			SystemHandleInformation, // 0x10
			SystemObjectInformation, // 0x11
			SystemPageFileInformation, // 0x12
			SystemVdmInstemulInformation, // 0x13
			SystemVdmBopInformation, // 0x14
			SystemFileCacheInformation, // 0x15
			SystemPoolTagInformation, // 0x16
			SystemInterruptInformation, // 0x17
			SystemDpcBehaviorInformation, // 0x18
			SystemFullMemoryInformation, // 0x19
			SystemLoadGdiDriverInformation, // 0x1A
			SystemUnloadGdiDriverInformation, // 0x1B
			SystemTimeAdjustmentInformation, // 0x1C
			SystemSummaryMemoryInformation, // 0x1D
			SystemNextEventIdInformation, // 0x1E
			SystemEventIdsInformation, // 0x1F
			SystemCrashDumpInformation, // 0x20
			SystemExceptionInformation, // 0x21
			SystemCrashDumpStateInformation, // 0x22
			SystemKernelDebuggerInformation, // 0x23
			SystemContextSwitchInformation, // 0x24
			SystemRegistryQuotaInformation, // 0x25
			SystemExtendServiceTableInformation, // 0x26
			SystemPrioritySeparation, // 0x27
			SystemPlugPlayBusInformation, // 0x28
			SystemDockInformation, // 0x29
			SystemPowerInformation, // 0x2A
			SystemProcessorSpeedInformation, // 0x2B
			// ---------- Windows 2000 ----------
			SystemCurrentTimeZoneInformation, // 0x2C
			SystemLookasideInformation, // 0x2D
			SystemTimeSlipNotification, // 0x2E
			SystemSessionCreate, // 0x2F
			SystemSessionDetach, // 0x30
			SystemSessionInformation, // 0x31
			SystemRangeStartInformation, // 0x32
			SystemVerifierInformation, // 0x33
			SystemVerifierAddDriverInformation, // 0x34
			SystemSessionProcessInformation, // 0x35
			// ---------- Windows 2000 SP4 ----------
			// ---------- Windows XP ----------1
			SystemLoadGdiDriverInSystemSpace, // 0x36
			SystemNumaProcessorMap, // 0x37
			// ---------- Windows XP ----------
			SystemPrefetcherInformation, // 0x38
			SystemExtendedProcessInformation, // 0x39
			SystemRecommendedSharedDataAlignment, // 0x3A
			SystemComPlusPackage, // 0x3B
			SystemNumaAvailableMemoryNode, // 0x3C
			SystemProcessorPowerInformation, // 0x3D
			SystemEmulationBasicInformation, // 0x3E
			SystemEmulationProcessorInformation, // 0x3F
			SystemExtendedHandleInformation, // 0x40
			SystemLostDelayedWriteInformation, // 0x41
			// ---------- Windows Server 2003 ----------
			SystemBigPoolInformation, // 0x42
			SystemSessionPoolTagInformation, // 0x43
			SystemSessionMappedViewInformation, // 0x44
			SystemHotpatchInformation, // 0x45
			SystemObjectSecurityMode, // 0x46
			SystemWatchdogTimerHandler, // 0x47
			// ---------- Windows Server 2003 SP1 ----------
			SystemWatchdogTimerInformation, // 0x48
			SystemLogicalProcessorInformation, // 0x49
			SystemWow64SharedInformation, // 0x4A
			SystemRegisterFirmwareTableInformationHandler, // 0x4B
			SystemFirmwareTableInformation, // 0x4C
			SystemModuleInformationEx, // 0x4D
			SystemVerifierTriageInformation, // 0x4E
			SystemSuperfetchInformation, // 0x4F
			SystemMemoryListInformation, // 0x50
			SystemFileCacheInformationEx, // 0x51
			// ---------- Windows Vista ----------
			SystemThreadPriorityClientIdInformation, //0x52
			SystemProcessorIdleCycleTimeInformation, //0x53
			SystemVerifierCancellationInformation, //0x54
			SystemProcessorPowerInformationEx, //0x55
			SystemRefTraceInformation, //0x56
			SystemSpecialPoolInformation, //0x57
			SystemProcessIdInformation, //0x58
			SystemErrorPortInformation, //0x59
			SystemBootEnvironmentInformation, //0x5A
			SystemHypervisorInformation, //0x5B
			SystemVerifierInformationEx, //0x5C
			SystemTimeZoneInformation, //0x5D
			SystemImageFileExecutionOptionsInformation, //0x5E
			SystemCoverageInformation, //0x5F
			SystemPrefetchPathInformation, //0x60
			SystemVerifierFaultsInformation, //0x61
			// -----------------------------------
			SystemSystemPartitionInformation,
			SystemSystemDiskInformation,
			SystemProcessorPerformanceDistribution,
			SystemNumaProximityNodeInformation,
			SystemDynamicTimeZoneInformation,
			SystemCodeIntegrityInformation,
			SystemProcessorMicrocodeUpdateInformation,
			SystemProcessorBrandString,
			SystemVirtualAddressInformation,
			SystemLogicalProcessorInformationEx,
			SystemProcessorCycleTimeInformation,
			SystemStoreInformation,
			SystemRegistryAppendStringInformation,
			SystemAitSamplingValue,
			SystemVhdBootInformation,
			SystemCpuQuotaInformation,
			SystemNotImplemented114,
			SystemNotImplemented115,
			SystemLowPriorityIoInformation,
			SystemTpmBootEntropyInformation,
			SystemVerifierInformation2,
			SystemPagedPoolWorkingSetInformation,
			SystemSystemPtesWorkingSetInformation,
			SystemNumaDistanceInformation,
			SystemSlicAuditResultsInformation,
			SystemBasicPerformanceInformation,
			SystemQueryPerformanceCounterInformation
		} SYSTEM_INFORMATION_CLASS;

		// 0xB8
		typedef struct _SYSTEM_PROCESS_INFORMATION
		{
			ULONG NextEntryOffset; // 0x00
			ULONG NumberOfThreads; // 0x04
			LARGE_INTEGER SpareLi1; // 0x08
			LARGE_INTEGER SpareLi2; // 0x10
			LARGE_INTEGER SpareLi3; // 0x18
			LARGE_INTEGER CreateTime; // 0x20
			LARGE_INTEGER UserTime; // 0x28
			LARGE_INTEGER KernelTime; // 0x30
			UNICODE_STRING ImageName; // 0x38
			KPRIORITY BasePriority; // 0x40
			DWORD UniqueProcessId; // 0x44 (changed HANDLE to DWORD)
			DWORD InheritedFromUniqueProcessId; // 0x48 (changed HANDLE to DWORD)
			ULONG HandleCount; // 0x4C
			union
			{
				// ---------- Windows NT4 ----------
				ULONG SpareUl0; // 0x50 
				// ---------- Windows 2000 ----------
				ULONG SessionId; // 0x50 
			};
			union
			{
				// ---------- Windows NT4 ----------
				// ---------- Windows 2000 ----------
				ULONG SpareUl1; // 0x54
				// ---------- Windows XP ----------
				PVOID PageDirectoryBase; // 0x54 (Windows XP+ using SystemExtendedProcessInformation - DirectoryTableBase[0] >> 0x0C)
			};
			ULONG PeakVirtualSize; // 0x58 (VM_COUNTERS)
			ULONG VirtualSize; // 0x5C
			ULONG PageFaultCount; // 0x60
			ULONG PeakWorkingSetSize; // 0x64
			ULONG WorkingSetSize; // 0x68
			ULONG QuotaPeakPagedPoolUsage; // 0x6C
			ULONG QuotaPagedPoolUsage; // 0x70
			ULONG QuotaPeakNonPagedPoolUsage; // 0x74
			ULONG QuotaNonPagedPoolUsage; // 0x78
			ULONG PagefileUsage; // 0x7C
			ULONG PeakPagefileUsage; // 0x80
			ULONG PrivatePageCount; // 0x84
			// ---------- Windows 2000 ----------
			LARGE_INTEGER ReadOperationCount; // 0x88 (IO_COUNTERS)
			LARGE_INTEGER WriteOperationCount; // 0x90
			LARGE_INTEGER OtherOperationCount; // 0x98
			LARGE_INTEGER ReadTransferCount; // 0xA0
			LARGE_INTEGER WriteTransferCount; // 0xA8
			LARGE_INTEGER OtherTransferCount; // 0xB0
		} SYSTEM_PROCESS_INFORMATION;

		// 0x0C
		typedef struct _SYSTEM_SESSION_PROCESS_INFORMATION
		{
			ULONG SessionId; // 0x00
			ULONG SizeOfBuf; // 0x04
			PVOID Buffer; // 0x08 (SYSTEM_PROCESS_INFORMATION*)
		} SYSTEM_SESSION_PROCESS_INFORMATION;

		// 0x3A
		typedef struct _SYSTEM_THREAD_INFORMATION
		{
			LARGE_INTEGER KernelTime; // 0x00
			LARGE_INTEGER UserTime; // 0x08
			LARGE_INTEGER CreateTime; // 0x10
			ULONG WaitTime; // 0x18
			PVOID StartAddress; // 0x1C
			CLIENT_ID ClientId; // 0x20
			KPRIORITY Priority; // 0x28
			LONG BasePriority; // 0x2C
			ULONG ContextSwitches; // 0x30
			ULONG ThreadState; // 0x34
			ULONG WaitReason; // 0x38
		} SYSTEM_THREAD_INFORMATION;

		typedef enum _PROCESS_INFORMATION_CLASS
		{
			ProcessBasicInformation, // 0x00
			ProcessQuotaLimits, // 0x01
			ProcessIoCounters, // 0x02
			ProcessVmCounters, // 0x03
			ProcessTimes, // 0x04
			ProcessBasePriority, // 0x05
			ProcessRaisePriority, // 0x06
			ProcessDebugPort, // 0x07
			ProcessExceptionPort, // 0x08
			ProcessAccessToken, // 0x09
			ProcessLdtInformation, // 0x0A
			ProcessLdtSize, // 0x0B
			ProcessDefaultHardErrorMode, // 0x0C
			ProcessIoPortHandlers, // 0x0D
			ProcessPooledUsageAndLimits, // 0x0E
			ProcessWorkingSetWatch, // 0x0F
			ProcessUserModeIOPL, // 0x10
			ProcessEnableAlignmentFaultFixup, // 0x11
			ProcessPriorityClass, // 0x12
			ProcessWx86Information, // 0x13
			ProcessHandleCount, // 0x14
			ProcessAffinityMask, // 0x15
			ProcessPriorityBoost, // 0x16
			// ---------- Windows 2000 ----------
			ProcessDeviceMap, // 0x17
			ProcessSessionInformation, // 0x18
			ProcessForegroundInformation, // 0x19 (STATUS_INVALID_INFO_CLASS)
			ProcessWow64Information, // 0x1A
			// ---------- Windows XP ----------
			ProcessImageFileName, // 0x1B
			ProcessLUIDDeviceMapsEnabled, // 0x1C
			ProcessBreakOnTermination, // 0x1D
			ProcessDebugObjectHandle, // 0x1E
			ProcessDebugFlags, // 0x1F
			ProcessHandleTracing, // 0x20
			// ---------- Windows XP SP2 ----------
			ProcessIoPriority, // 0x21 (STATUS_INVALID_INFO_CLASS)
			ProcessExecuteFlags, // 0x22
			ProcessResourceManagement, // 0x23 (STATUS_INVALID_INFO_CLASS)
			ProcessCookie, // 0x24
			ProcessImageInformation, // 0x25
			// ---------- Windows Vista ----------
			ProcessCycleTime, // 0x26
			ProcessPagePriority,  // 0x27
			ProcessInstrumentationCallback, // 0x28
			// -----------------------------------
			ProcessThreadStackAllocation,
			ProcessWorkingSetWatchEx,
			ProcessImageFileNameWin32,
			ProcessImageFileMapping,
			ProcessAffinityUpdateMode,
			ProcessMemoryAllocationMode,
			ProcessGroupInformation,
			ProcessTokenVirtualizationEnabled,
			ProcessConsoleHostProcess,
			ProcessWindowInformation
		} PROCESS_INFORMATION_CLASS;

		// 0x18
		typedef struct _PROCESS_BASIC_INFORMATION
		{
			NTSTATUS ExitStatus; // 0x00
			PEB* PebBaseAddress; // 0x04
			ULONG_PTR AffinityMask; // 0x08
			KPRIORITY BasePriority; // 0x0C
			ULONG_PTR UniqueProcessId; // 0x10
			ULONG_PTR InheritedFromUniqueProcessId; // 0x14
		} PROCESS_BASIC_INFORMATION;

		// 0x04
		typedef struct _PROCESS_SESSION_INFORMATION
		{
			DWORD SessionId; // 0x00
		} PROCESS_SESSION_INFORMATION;

		// 0x18
		typedef struct _OBJECT_ATTRIBUTES
		{
			ULONG Length; // 0x00
			HANDLE RootDirectory; // 0x04
			UNICODE_STRING* ObjectName; // 0x08
			ULONG Attributes; // 0x0C
			PVOID SecurityDescriptor; // 0x10
			PVOID SecurityQualityOfService; // 0x04
		} OBJECT_ATTRIBUTES;

		typedef NTSTATUS (NTAPI* NTQUERYSYSTEMINFORMATION)(SYSTEM_INFORMATION_CLASS pSystemInformationClass, PVOID pSystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
		typedef NTSTATUS (NTAPI* NTQUERYINFORMATIONPROCESS)(HANDLE hProcess, PROCESS_INFORMATION_CLASS pProcessInformationClass, PVOID pInformation, ULONG pInformationLength, PULONG pReturnLength);
		typedef NTSTATUS (NTAPI* NTOPENTHREAD)(HANDLE* hThread, DWORD dwDesiredAccess, OBJECT_ATTRIBUTES* pObjectAttributes, CLIENT_ID* dwThreadId);
		typedef NTSTATUS (NTAPI* RTLADJUSTPRIVILEGE)(DWORD dwPrivilege, BOOL bSet, BOOL bAdjustThread, BOOL* bWasSet);
		typedef BOOL (NTAPI* RTLINITIALIZECRITICALSECTIONANDSPINCOUNT)(CRITICAL_SECTION* lpCriticalSection, DWORD dwSpinCount);
		typedef VOID (NTAPI* RTLENABLEEARLYCRITICALSECTIONEVENTCREATION)(VOID);

	private:
		NTQUERYSYSTEMINFORMATION NtQuerySystemInformation;
		NTQUERYINFORMATIONPROCESS NtQueryInformationProcess;
		NTOPENTHREAD NtOpenThread;
		RTLADJUSTPRIVILEGE RtlAdjustPrivilege;
		RTLINITIALIZECRITICALSECTIONANDSPINCOUNT RtlInitializeCriticalSectionAndSpinCount;
		RTLENABLEEARLYCRITICALSECTIONEVENTCREATION RtlEnableEarlyCriticalSectionEventCreation;

	public:
		void setNtQuerySystemInformation(void* farPtr) { NtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)farPtr; }
		void setNtQueryInformationProcess(void* farPtr) { NtQueryInformationProcess = (NTQUERYINFORMATIONPROCESS)farPtr; }
		void setNtOpenThread(void* farPtr) { NtOpenThread = (NTOPENTHREAD)farPtr; }
		void setRtlAdjustPrivilege(void* farPtr) { RtlAdjustPrivilege = (RTLADJUSTPRIVILEGE)farPtr; }
		void setRtlInitializeCriticalSectionAndSpinCount(void* farPtr) { RtlInitializeCriticalSectionAndSpinCount = (RTLINITIALIZECRITICALSECTIONANDSPINCOUNT)farPtr; }
		void setRtlEnableEarlyCriticalSectionEventCreation(void* farPtr) { RtlEnableEarlyCriticalSectionEventCreation = (RTLENABLEEARLYCRITICALSECTIONEVENTCREATION)farPtr; }

		NTQUERYSYSTEMINFORMATION getNtQuerySystemInformation(void) { return NtQuerySystemInformation; }
		NTQUERYINFORMATIONPROCESS getNtQueryInformationProcess(void) { return NtQueryInformationProcess; }
		NTOPENTHREAD getNtOpenThread(void) { return NtOpenThread; }
		RTLADJUSTPRIVILEGE getRtlAdjustPrivilege(void) { return RtlAdjustPrivilege; }
		RTLINITIALIZECRITICALSECTIONANDSPINCOUNT getRtlInitializeCriticalSectionAndSpinCount(void) { return RtlInitializeCriticalSectionAndSpinCount; }
		RTLENABLEEARLYCRITICALSECTIONEVENTCREATION getRtlEnableEarlyCriticalSectionEventCreation(void) { return RtlEnableEarlyCriticalSectionEventCreation; }
};