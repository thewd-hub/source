#include <windows.h>

// unreferenced formal parameter
#pragma warning(disable:4100)

/////////////////
// entry point //
/////////////////
BOOL WINAPI DllMain(HINSTANCE hDLL, DWORD dwReason, LPVOID dwReserved)
{
	return TRUE;
}

///////////////////////////////////////////////
// check registration code for correct value //
///////////////////////////////////////////////
BOOL isRegisterationCodeAccepted(char* registrationCode)
{
	Sleep(200);
	return (BOOL)(_strcmpi(registrationCode, "Hello World") == 0);
}