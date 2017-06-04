//////////////////////////////////////////////////
// Support routines the process patching engine //
//////////////////////////////////////////////////

bool checkFileSizeMatch(unsigned long fileSize, PATCHINGDETAILS_FILEBASE* pFileBaseDetails);

bool hasProcessTerminated(HANDLE processHandle);
bool hasProcessTerminated(unsigned long processId);
bool hasThreadTerminated(HANDLE threadHandle);
void resumeThread(HANDLE threadHandle);

void handleProcessPatchingFatalError(int errorNumber);

bool attemptWin9xStealthMode(unsigned long processId);
DWORD getObfuscatorValue_9x(void);

unsigned long getPrimaryThreadId_9x(unsigned long processId);
HANDLE getPrimaryThreadHandle_9x(HANDLE processHandle, unsigned long threadId);

unsigned long getPrimaryThreadId_NTx(API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION* pSystemProcessInformation);
HANDLE getPrimaryThreadHandle_NTx(unsigned long processId, unsigned long threadId);

bool getSEDebugPrivilege(void);

DWORD getProcessSessionId(DWORD processId);

bool getPebInformation(HANDLE processHandle, PVOID* pPebLdr = NULL, PVOID* dwImageBaseAddress = NULL);