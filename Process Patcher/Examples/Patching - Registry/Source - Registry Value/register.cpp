#include <windows.h>
#include "resource.h"

// unreferenced formal parameter
#pragma warning(disable:4100)

/////////////////////////////////////
// has the registry key been found //
/////////////////////////////////////
BOOL IsRegistryKeyFound(void)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, "PROCESSPATCHERTEST\\KEY1", 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS) return FALSE;

	unsigned char* valueString = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 100);
	unsigned long valueType = 0;
	unsigned long valueStringLength = 100;

	RegQueryValueEx(hKey, "VALUE1", 0, &valueType, valueString, &valueStringLength);
	RegCloseKey(hKey);

	if (strcmp((char*)valueString, "REGISTERED") != 0)
	{
		HeapFree(GetProcessHeap(), NULL, valueString);
		return FALSE;
	}

	HeapFree(GetProcessHeap(), NULL, valueString);
	return TRUE;
}

/////////////////////////////
// delete the registry key //
/////////////////////////////
VOID DeleteRegistryKey(void)
{
	HKEY hKey;
	RegOpenKeyEx(HKEY_CURRENT_USER, "PROCESSPATCHERTEST\\KEY1", 0, KEY_ALL_ACCESS, &hKey);
	RegDeleteValue(hKey, "VALUE1");
	RegCloseKey(hKey);
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