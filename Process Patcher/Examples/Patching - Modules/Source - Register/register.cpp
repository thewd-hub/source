#include <windows.h>
#include "resource.h"

// unreferenced formal parameter
#pragma warning(disable:4100)

typedef BOOL (* ISREGISTERATIONCODEACCEPTED)(LPSTR);
ISREGISTERATIONCODEACCEPTED isRegisterationCodeAccepted1 = NULL;
ISREGISTERATIONCODEACCEPTED isRegisterationCodeAccepted2 = NULL;
HMODULE registerModule1 = NULL;
HMODULE registerModule2 = NULL;
bool isPluginControlled = false;

/////////////////////////////////
// main dialog message handler //
/////////////////////////////////
LRESULT CALLBACK MainDialogMessageHandler(HWND hDlgWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			break;
		}
		case WM_COMMAND:
		{
			if ((wParam == IDOK) || (wParam == IDCANCEL))
			{
				EndDialog(hDlgWnd, TRUE);
			}
			else if (wParam == IDC_BUTTON_REGISTER1)
			{
				char* registrationCode = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 30);
				GetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), registrationCode, 20);
				if ((isRegisterationCodeAccepted1 != NULL) && (isRegisterationCodeAccepted1(registrationCode))) SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), "Registration Succeeded - 1");
				else SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), "Registration Failed - 1");
				HeapFree(GetProcessHeap(), NULL, registrationCode);
			}
			else if (wParam == IDC_BUTTON_REGISTER2)
			{
				// load module 2
				if (registerModule2 == NULL) registerModule2 = LoadLibraryEx("register2.dll", NULL, NULL);
				if (isRegisterationCodeAccepted2 == NULL) isRegisterationCodeAccepted2 = (ISREGISTERATIONCODEACCEPTED)GetProcAddress(registerModule2, "isRegisterationCodeAccepted");

				char* registrationCode = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 30);
				GetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), registrationCode, 20);
				if ((isRegisterationCodeAccepted2 != NULL) && (isRegisterationCodeAccepted2(registrationCode))) SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), "Registration Succeeded - 2");
				else SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), "Registration Failed - 2");
				HeapFree(GetProcessHeap(), NULL, registrationCode);

				// don't free module when controlled by plugin
				if (!isPluginControlled)
				{
					if (isRegisterationCodeAccepted2 != NULL) isRegisterationCodeAccepted2 = NULL;
					if (registerModule2 != NULL)
					{
						FreeLibrary(registerModule2);
						registerModule2 = NULL;
					}
				}
			}
		}
	}

	return FALSE;
}

////////////////////////////
// main program execution //
////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// load module 1
	registerModule1 = LoadLibraryEx("register1.dll", NULL, NULL);
	isRegisterationCodeAccepted1 = (ISREGISTERATIONCODEACCEPTED)GetProcAddress(registerModule1, "isRegisterationCodeAccepted");

	// is under plugin control
	if ((lpCmdLine != NULL) && (strlen(lpCmdLine) > 0) && (strstr(lpCmdLine, "PLUGINCONTROL") != NULL)) isPluginControlled = true;

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_REGISTER_DIALOG), GetDesktopWindow(), (DLGPROC)MainDialogMessageHandler);

	if (registerModule1 != NULL) FreeLibrary(registerModule1);
	if (registerModule2 != NULL) FreeLibrary(registerModule2);
	return ERROR_SUCCESS;
}