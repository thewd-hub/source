#include <windows.h>

// unreferenced formal parameter
#pragma warning(disable:4100)

const char REGISTERfilename[] = "register.exe";
const char REGISTER1_DLLfilename[] = "register1.dll";
const char REGISTER2_DLLfilename[] = "register2.dll";

/////////////////////////////////////////////
// extracts the register.exe from the      //
// resource table to the current directory //
/////////////////////////////////////////////
bool extractREGISTERResourceFromFile(void)
{
	HRSRC findResourceHandle;
	HGLOBAL resourceHandle;

	findResourceHandle = FindResource(NULL, "REGISTER", RT_RCDATA);
	if (findResourceHandle == NULL) return false;
	unsigned int resourceSize = SizeofResource(NULL, findResourceHandle);
	resourceHandle = LoadResource(NULL, findResourceHandle);
	if (resourceHandle == NULL) return false;

	unsigned char* registerResource = (unsigned char*)LockResource(resourceHandle);
	unsigned long actuallyWritten;

	// write the resource REGISTER to the file register.exe
	HANDLE fileHandle = CreateFile(REGISTERfilename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE) return false;
	WriteFile(fileHandle, registerResource, resourceSize, &actuallyWritten, NULL);
	CloseHandle(fileHandle);

	return (actuallyWritten == resourceSize);
}

/////////////////////////////////////////////
// extracts the register1.dll from the     //
// resource table to the current directory //
/////////////////////////////////////////////
bool extractREGISTER1_DLLResourceFromFile(void)
{
	HRSRC findResourceHandle;
	HGLOBAL resourceHandle;

	findResourceHandle = FindResource(NULL, "REGISTER1_DLL", RT_RCDATA);
	if (findResourceHandle == NULL) return false;
	unsigned int resourceSize = SizeofResource(NULL, findResourceHandle);

	resourceHandle = LoadResource(NULL, findResourceHandle);
	if (resourceHandle == NULL) return false;

	unsigned char* registerResource = (unsigned char*)LockResource(resourceHandle);
	unsigned long actuallyWritten;

	// write the resource REGISTER_DLL to the file register1.dll
	HANDLE fileHandle = CreateFile(REGISTER1_DLLfilename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE) return false;
	WriteFile(fileHandle, registerResource, resourceSize, &actuallyWritten, NULL);
	CloseHandle(fileHandle);

	return (actuallyWritten == resourceSize);
}

/////////////////////////////////////////////
// extracts the register2.dll from the     //
// resource table to the current directory //
/////////////////////////////////////////////
bool extractREGISTER2_DLLResourceFromFile(void)
{
	HRSRC findResourceHandle;
	HGLOBAL resourceHandle;

	findResourceHandle = FindResource(NULL, "REGISTER2_DLL", RT_RCDATA);
	if (findResourceHandle == NULL) return false;
	unsigned int resourceSize = SizeofResource(NULL, findResourceHandle);

	resourceHandle = LoadResource(NULL, findResourceHandle);
	if (resourceHandle == NULL) return false;

	unsigned char* registerResource = (unsigned char*)LockResource(resourceHandle);
	unsigned long actuallyWritten;

	// write the resource REGISTER_DLL to the file register2.dll
	HANDLE fileHandle = CreateFile(REGISTER2_DLLfilename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE) return false;
	WriteFile(fileHandle, registerResource, resourceSize, &actuallyWritten, NULL);
	CloseHandle(fileHandle);

	return (actuallyWritten == resourceSize);
}

/////////////////////////////////////////////////////
// delete the resources from the current directory //
/////////////////////////////////////////////////////
void removeResourcesFromSystem(void)
{
	DeleteFile(REGISTERfilename);
	DeleteFile(REGISTER1_DLLfilename);
	DeleteFile(REGISTER2_DLLfilename);
}

////////////////////////////
// main program execution //
////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// extract resources to the current directory
	if ((!extractREGISTERResourceFromFile()) ||
		(!extractREGISTER1_DLLResourceFromFile()) ||
		(!extractREGISTER2_DLLResourceFromFile()))
	{
		MessageBox(NULL, "Error occurred extracting the Child Process to directory", "CreatesChildProcess - Error", MB_OK);
		ExitProcess(1);
	}

	PROCESS_INFORMATION processInfo;
	STARTUPINFO startupInfo;

	ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.lpReserved = NULL;
	startupInfo.lpReserved2 = NULL;
	startupInfo.cbReserved2 = 0;

	// create the child process
	if (CreateProcess(REGISTERfilename, NULL, NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP | NORMAL_PRIORITY_CLASS,
		NULL, NULL,	&startupInfo, &processInfo) == 0)
	{
		MessageBox(NULL, "Unable to execute the Child Process", "CreatesChildProcess - Error", MB_OK);
		ExitProcess(1);
	}

	// wait for the child process to terminate
	WaitForSingleObject(processInfo.hProcess, INFINITE);

	CloseHandle(processInfo.hThread);
	CloseHandle(processInfo.hProcess);

	// remove resources from the current directory
	removeResourcesFromSystem();
	return ERROR_SUCCESS;
}