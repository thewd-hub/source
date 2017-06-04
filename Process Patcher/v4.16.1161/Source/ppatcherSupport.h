///////////////////////////////////////////////
// Support functions for the process patcher //
///////////////////////////////////////////////

void displayScriptInformation(void);

void displayConsoleMessage(char* debugMessage, bool createNewLine = true, bool isDebugMessage = false);

void showProcessPatcherWindow(bool forceShowWindow = false);
void hideProcessPatcherWindow(void);
bool isProcessPatcherWindowHidden(void);

void adjustConsoleScreenBufferSize(unsigned short numberOfRows);
void resetConsoleScreenBufferSize(void);

bool isTargetProcessPatcher(void);