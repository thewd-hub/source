////////////////////////////////////////////////////////
// Searches for information about executing processes //
////////////////////////////////////////////////////////

typedef struct _PROCESS_INFORMATION_EX
{
	HANDLE hProcess;
	HANDLE hThread;
	DWORD dwProcessId;
	DWORD dwThreadId;
	LPSTR lpFilename;
	DWORD dwSessionId;
	PVOID pPebLdrAddress;
	PVOID pImageBaseAddress;
} PROCESS_INFORMATION_EX;

bool initialiseProcessPatching(void);
bool uninitialiseProcessPatching(void);

PROCESS_INFORMATION_EX* getProcessInformationEx(char* fileName, unsigned long processId, HANDLE processHandle = NULL, bool getThreadInformation = false, API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION* pSystemProcessInformation = NULL);

bool getAllProcessInformation(PROCESS_INFORMATION_EX* pInformationEx[], int* numberOfProcesses, int maximumProcesses);

PROCESS_INFORMATION_EX* findProcessInformation(char* shortProcessFilename, PATCHINGDETAILS_FILEBASE* pFileBaseDetails = NULL, bool findChildProcess = false, unsigned long parentProcessId = 0, unsigned long selectedProcessId = 0);
PROCESS_INFORMATION_EX* findProcessInformation(PATCHINGDETAILS_CHILDPROCESS* pChildProcessDetails);
PROCESS_INFORMATION_EX* findProcessInformation(unsigned long parentProcessId);