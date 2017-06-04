#include <windows.h>
#include "resource.h"

// unreferenced formal parameter
#pragma warning(disable:4100)

// registration code accepted
BOOL isRegistered = FALSE;

///////////////////////////////////////////////
// check registration code for correct value //
///////////////////////////////////////////////
BOOL isRegisterationCodeAccepted(char* registrationCode)
{
	return (BOOL)(_strcmpi(registrationCode, "Hello World") == 0);
}

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
			if ((wParam == IDOK) || (wParam == IDCANCEL) || ((isRegistered) && (wParam == IDC_BUTTON_REGISTER)))
			{
				EndDialog(hDlgWnd, TRUE);
			}
			else if (wParam == IDC_BUTTON_REGISTER)
			{
				char* registrationCode = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 30);
				GetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), registrationCode, 20);
				if (isRegisterationCodeAccepted(registrationCode))
				{
					SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), "Registration Succeeded");
					SetWindowText(GetDlgItem(hDlgWnd, IDC_BUTTON_REGISTER), "Exit");
					PostMessage(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), EM_SETREADONLY, (WPARAM)TRUE, (LPARAM)0);
					isRegistered = TRUE;
				}

				HeapFree(GetProcessHeap(), NULL, registrationCode);
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
	// display arguments (if supplied)
	if ((lpCmdLine != NULL) && (strlen(lpCmdLine) > 0))
	{
		// display process & thread ids
		if (strstr(lpCmdLine, "DISPLAY_IDS") != NULL)
		{
			char* outputMessage = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);
			wsprintf(outputMessage, "Process Id - 0x%X\r\nThread Id - 0x%X", GetCurrentProcessId(), GetCurrentThreadId());
			MessageBox(NULL, outputMessage, "Process Patcher Example", MB_OK | MB_ICONINFORMATION);
			HeapFree(GetProcessHeap(), NULL, outputMessage);
		}
		// otherwise, display all the arguments
		else
		{
			MessageBox(NULL, lpCmdLine, "Process Patcher Example - Arguments", MB_OK);
		}
	}

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_REGISTER_DIALOG), GetDesktopWindow(), (DLGPROC)MainDialogMessageHandler);
	return ERROR_SUCCESS;
}