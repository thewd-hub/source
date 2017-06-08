///////////////////////////////////////////////////////////
// Keyboard Input Simulator                              //
// (C)thewd, thewd@hotmail.com                           //
///////////////////////////////////////////////////////////

#define INPUT_MOUSE		0
#define INPUT_KEYBOARD	1
#define INPUT_HARDWARE	2

#define	KEYEVENTF_EXTENDEDKEY	0x0001
#define	KEYEVENTF_KEYUP			0x0002
#define	KEYEVENTF_UNICODE		0x0004
#define	KEYEVENTF_SCANCODE		0x0008

#ifndef _WINUSER_
typedef struct _MOUSEINPUT
{
	LONG dx;
	LONG dy;
	DWORD mouseData;
	DWORD dwFlags;
	DWORD time;
	ULONG_PTR dwExtraInfo;
} MOUSEINPUT;

typedef struct _KEYBDINPUT
{
	WORD wVk;
	WORD wScan;
	DWORD dwFlags;
	DWORD time;
	ULONG_PTR dwExtraInfo;
} KEYBDINPUT;

typedef struct _HARDWAREINPUT
{
	DWORD uMsg;
	WORD wParamL;
	WORD wParamH;
} HARDWAREINPUT;

typedef struct _INPUT
{
	DWORD type;
	union
	{
		MOUSEINPUT mi;
		KEYBDINPUT ki;
		HARDWAREINPUT hi;
	};
} INPUT;
#endif

typedef UINT (WINAPI* SENDINPUT)(UINT nInputs, INPUT* pInputs, int cbSize);
typedef BOOL (WINAPI* GETGUITHREADINFO)(DWORD idThread, PGUITHREADINFO pThreadInfo);