//////////////////////////////////////////////////////////////////////
// Handles the patching of the target process and any child process //
//////////////////////////////////////////////////////////////////////

bool initialiseTargetProcessPatching(void);
bool uninitialiseTargetProcessPatching(void);

void executeTargetProcessPatchingThread(LPVOID args);

char* getTargetProcessFilename(void);
unsigned long getTargetProcessId(void);
unsigned long getTargetProcessThreadId(void);
HANDLE getTargetProcessHandle(void);
HANDLE getTargetProcessThreadHandle(void);
unsigned long getTargetProcessSessionId(void);

char* getChildProcessFilename(void);
unsigned long getChildProcessId(void);
unsigned long getChildProcessThreadId(void);
HANDLE getChildProcessHandle(void);
HANDLE getChildProcessThreadHandle(void);
unsigned long getChildProcessSessionId(void);

char* getMainProcessFilename(void);
unsigned long getMainProcessId(void);
unsigned long getMainProcessThreadId(void);
HANDLE getMainProcessHandle(void);
HANDLE getMainProcessThreadHandle(void);

PVOID getMainProcessPebLdrAddress(void);
PVOID getMainProcessImageBaseAddress(void);