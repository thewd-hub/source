#include <windows.h>
#include "resource.h"

// unreferenced formal parameter
#pragma warning(disable:4100)

/////////////////////////////////////
// had the registry key been found //
/////////////////////////////////////
BOOL IsRegistryKeyFound(void)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, "PROCESSPATCHERTEST\\KEY1", 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS) return FALSE;
	RegCloseKey(hKey);
	return TRUE;
}

/////////////////////////////
// delete the registry key //
/////////////////////////////
VOID DeleteRegistryKey(void)
{
	RegDeleteKey(HKEY_CURRENT_USER, "PROCESSPATCHERTEST\\KEY1");
	RegDeleteKey(HKEY_CURRENT_USER, "PROCESSPATCHERTEST");
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
			if (IsRegistryKeyFound()) SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), "Succeeded");
			else SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), "Failed");
			break;
		}
		case WM_COMMAND:
		{
			if ((wParam == IDOK) || (wParam == IDCANCEL))
			{
				EndDialog(hDlgWnd, TRUE);
			}
			else if (wParam == IDC_BUTTON_RETRY)
			{
				if (IsRegistryKeyFound()) SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), "Succeeded");
				else SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), "Failed");
			}
			else if (wParam == IDC_BUTTON_REMOVE)
			{
				DeleteRegistryKey();
				if (IsRegistryKeyFound()) SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), "Succeeded");
				else SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT_REGCODE), "Failed");
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
	if (strlen(lpCmdLine) > 0) MessageBox(NULL, lpCmdLine, "Process Patcher Example - Arguments", MB_OK);
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_REGISTER_DIALOG), GetDesktopWindow(), (DLGPROC)MainDialogMessageHandler);
	return ERROR_SUCCESS;
}