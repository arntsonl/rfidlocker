// RfidLocker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "RfidLocker.h"
#include "RfidUtils.h"
#include <CommCtrl.h>

#pragma comment(linker, \
	"\"/manifestdependency:type='Win32' "\
	"name='Microsoft.Windows.Common-Controls' "\
	"version='6.0.0.0' "\
	"processorArchitecture='*' "\
	"publicKeyToken='6595b64144ccf1df' "\
	"language='*'\"")

#pragma comment(lib, "ComCtl32.lib")


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
NOTIFYICONDATA niData;

// Forward declarations of functions included in this code module:
BOOL                InitInstance(HINSTANCE, int);
INT_PTR CALLBACK    DialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ReadTag(HWND, UINT, WPARAM, LPARAM);
ULONGLONG			GetDllVersion(LPCTSTR lpszDllName);
void				ShowContextMenu(HWND, DWORD, DWORD);
DWORD WINAPI		CheckRfidThread(LPVOID);
static int g_rfidReadMode = 0;
static HWND g_readRfidHwnd;
static HWND g_parentHwnd;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	// Ensure that the common control DLL is loaded, and then create 
	// the header control. 
	INITCOMMONCONTROLSEX icex;  //declare an INITCOMMONCONTROLSEX Structure
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES;   //set dwICC member to ICC_LISTVIEW_CLASSES    
	// this loads list-view and header control classes.
	InitCommonControlsEx(&icex);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RFIDLOCKER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

VOID AddTrayIcon(HWND hWnd)
{

	// zero the structure - note: Some Windows funtions
	// require this but I can't be bothered to remember
	// which ones do and which ones don't.
	ZeroMemory(&niData, sizeof(NOTIFYICONDATA));

	// get Shell32 version number and set the size of the
	// structure note: the MSDN documentation about this is
	// a little dubious(see bolow) and I'm not at all sure
	// if the code bellow is correct
	ULONGLONG ullVersion = GetDllVersion(_T("Shell32.dll"));

	if (ullVersion >= MAKEDLLVERULL(6, 0, 0, 0))
		niData.cbSize = sizeof(NOTIFYICONDATA);
	else if (ullVersion >= MAKEDLLVERULL(5, 0, 0, 0))
		niData.cbSize = NOTIFYICONDATA_V2_SIZE;
	else niData.cbSize = NOTIFYICONDATA_V1_SIZE;


	// the ID number can be any UINT you choose and will
	// be used to identify your icon in later calls to
	// Shell_NotifyIcon
	niData.uID = IDC_MYICON;

	// state which structure members are valid
	// here you can also choose the style of tooltip
	// window if any - specifying a balloon window:
	// NIF_INFO is a little more complicated 
	niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

	// load the icon note: you should destroy the icon
	// after the call to Shell_NotifyIcon
	niData.hIcon =
		(HICON)LoadImage(hInst,
			MAKEINTRESOURCE(IDI_RFIDLOCKER),
			IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON),
			GetSystemMetrics(SM_CYSMICON),
			LR_DEFAULTCOLOR);

	// set the window you want to recieve event messages
	niData.hWnd = hWnd;

	// set the message to send
	// note: the message value should be in the
	// range of WM_APP through 0xBFFF
	niData.uCallbackMessage = IDM_ICON_TRAY_MESSAGE;

	lstrcpy(niData.szTip, L"Rfid Locker");

	// NIM_ADD adds a new tray icon
	Shell_NotifyIcon(NIM_ADD, &niData);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	InitRfidLocker();
	InitArduino();

	HWND hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN), 0, DialogProc, 0);

	if (!hDlg)
	{
		return FALSE;
	}

	// Fill in the COM ports
	HWND comItem = GetDlgItem(hDlg, IDC_COM_SELECT);
	SendMessage(comItem, CB_ADDSTRING, 0, (LPARAM)L"COM0");
	SendMessage(comItem, CB_ADDSTRING, 0, (LPARAM)L"COM1");
	SendMessage(comItem, CB_ADDSTRING, 0, (LPARAM)L"COM2");
	SendMessage(comItem, CB_ADDSTRING, 0, (LPARAM)L"COM3");
	SendMessage(comItem, CB_ADDSTRING, 0, (LPARAM)L"COM4");
	SendMessage(comItem, CB_ADDSTRING, 0, (LPARAM)L"COM5");
	SendMessage(comItem, CB_ADDSTRING, 0, (LPARAM)L"COM6");
	SendMessage(comItem, CB_ADDSTRING, 0, (LPARAM)L"COM7");
	SendMessage(comItem, CB_ADDSTRING, 0, (LPARAM)L"COM8");
	SendMessage(comItem, CB_ADDSTRING, 0, (LPARAM)L"COM9");
	SendMessage(comItem, CB_SETCURSEL, (WPARAM)g_rfidOptions.comNumber, 0);

	// Fill the read id
	
	HWND readHwnd = GetDlgItem(hDlg, IDC_READ_ID);
	if (strcmp(g_rfidOptions.readTag, "") == 0)
	{
		SendMessage(readHwnd, WM_SETTEXT, 0, (LPARAM)L"NONE");
	}
	else
	{
		TCHAR buffer_t[2048];
		mbstowcs(buffer_t, g_rfidOptions.readTag, 1024);
		SendMessage(readHwnd, WM_SETTEXT, 0, (LPARAM)buffer_t);
	}

	// Fill the frequency
	HWND sliderHwnd = GetDlgItem(hDlg, IDC_FREQUENCY_SLIDER);
	SendMessage(sliderHwnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELPARAM(1, 20));
	SendMessage(sliderHwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)g_rfidOptions.frequency);

	DWORD threadId;
	CreateThread(NULL, 0, CheckRfidThread, NULL, 0, &threadId);

	// If this is the first start, show the window, otherwise hide on boot
	if (strcmp(g_rfidOptions.readTag, "") == 0)
		ShowWindow(hDlg, nCmdShow);
	else
		ShowWindow(hDlg, SW_HIDE);
	UpdateWindow(hDlg);
	AddTrayIcon(hDlg);

	return TRUE;
}

// Message handler for about box.
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND tmpWnd;
	int selNumber;

	switch (message)
	{
	case WM_HSCROLL:
		tmpWnd = GetDlgItem(hDlg, IDC_FREQUENCY_SLIDER);
		selNumber = SendMessage(tmpWnd, TBM_GETPOS, 0, 0);
		SetFrequency(selNumber);
		break;
	case IDM_ICON_TRAY_MESSAGE:
		switch (lParam)
		{
		case WM_LBUTTONDBLCLK:
			ShowWindow(hDlg, SW_RESTORE);
			BringWindowToTop(hDlg);
			break;
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			ShowContextMenu(hDlg, lParam, wParam);
		}
		break;
	case WM_INITDIALOG:
		g_parentHwnd = hDlg;
		return (INT_PTR)TRUE;

	case WM_CLOSE: /* there are more things to go here, */
		DestroyWindow(hDlg);
		return TRUE; /* just continue reading on... */

	case WM_SIZE:
		switch (wParam)
		{
		case SIZE_MINIMIZED:
			ShowWindow(hDlg, SW_HIDE);
		}
		return TRUE;

	case WM_DESTROY:
		Shell_NotifyIcon(NIM_DELETE, &niData);
		PostQuitMessage(0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_COM_SELECT:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				tmpWnd = GetDlgItem(hDlg, IDC_COM_SELECT);
				selNumber = SendMessage(tmpWnd, CB_GETCURSEL, 0, 0); //Global variable
				ChangeCOMPort(selNumber);
				break;
			}
			break;
		case IDC_READ_NEW_TAG:
			g_rfidReadMode = 2;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_READ_TAG), hDlg, (DLGPROC)ReadTag);
			break;
		case IDC_SAVE_BUTTON:
			SaveRfidSettings();
			break;
		case IDM_EXIT:
			DestroyWindow(hDlg);
			return TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Message handler for reading tag.
INT_PTR CALLBACK ReadTag(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		g_readRfidHwnd = hDlg;
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			g_rfidReadMode = 1;
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void ShowContextMenu(HWND hWnd, DWORD lParam, DWORD wParam)
{
	POINT cursor;
	GetCursorPos(&cursor);
	HMENU hMenu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_POPUP_MENU));
	hMenu = GetSubMenu(hMenu, 0);
	TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, cursor.x, cursor.y, 0, hWnd, NULL);
}

#ifndef __WIN32OS2__
// Get dll version number
ULONGLONG GetDllVersion(LPCTSTR lpszDllName)
{
	ULONGLONG ullVersion = 0;
	HINSTANCE hinstDll;
	hinstDll = LoadLibrary(lpszDllName);
	if (hinstDll)
	{
		DLLGETVERSIONPROC pDllGetVersion;
		pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");
		if (pDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			HRESULT hr;
			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);
			hr = (*pDllGetVersion)(&dvi);
			if (SUCCEEDED(hr))
				ullVersion = MAKEDLLVERULL(dvi.dwMajorVersion, dvi.dwMinorVersion, 0, 0);
		}
		FreeLibrary(hinstDll);
	}
	return ullVersion;
}
#endif

DWORD WINAPI CheckRfidThread(LPVOID lParam)
{
	char buffer[2048];
	TCHAR buffer_t[2048];
	HWND tmpHwnd;
	while (1)
	{
		// Sleep our frequency
		Sleep(g_rfidOptions.frequency*10 + 1000);
		
		if (g_SP->IsConnected() == true)
		{
			char * tag = GetTag();
			if (g_rfidReadMode == 0 && strlen(tag) > 0)
			{
				g_SP->WriteData(ARDUINO_SET_TAG, 1);
				Sleep(50);
				g_SP->WriteData(tag, strlen(tag));
				Sleep(50);
				int bytesRead = g_SP->ReadData(buffer, 2048);
				if (bytesRead > 0)
				{
					buffer[bytesRead] = 0;
					if (strcmp(buffer, "1") == 0)
					{
						g_rfidReadMode = 1;
					}
				}
			}
			else if (g_rfidReadMode == 1)
			{
				g_SP->WriteData(ARDUINO_POLL_TAG, 1);
				Sleep(50);
				int bytesRead = g_SP->ReadData(buffer, 2048);
				if (bytesRead > 0)
				{
					buffer[bytesRead] = 0;
					if (strcmp(buffer, CARD_PRESENT) == 0)
					{
						tmpHwnd = GetDlgItem(g_parentHwnd, IDC_STATUS_TEXT);
						SendMessage(tmpHwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)L"Status: Active");
					}
					// Check to see if we've disconnected
					else if (strcmp(buffer, CARD_NOT_PRESENT) == 0)
					{
						// Lock the machine!!
						PROCESS_INFORMATION pi = { 0 };
						STARTUPINFOA si = { 0 };
						si.cb = sizeof(si);
						char * szCmdline = "rundll32.exe user32.dll,LockWorkStation";
						CreateProcessA(NULL, szCmdline, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
					}
				}
			}
			else if (g_rfidReadMode == 2)
			{
				g_SP->WriteData(ARDUINO_READ_TAG, 1);
				Sleep(50);
				int bytesRead = g_SP->ReadData(buffer, 2048);
				if (bytesRead > 0)
				{
					buffer[bytesRead] = 0;
					if (strlen(buffer) > 1)
					{
						// Got a UUID!
						ReadNewTag(buffer);
						tmpHwnd = GetDlgItem(g_parentHwnd, IDC_READ_ID);
						mbstowcs(buffer_t, buffer, 1024);
						SendMessage(tmpHwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)buffer_t);
						g_rfidReadMode = 0;
						EndDialog(g_readRfidHwnd, 0);
					}
				}
			}
		}
	}
	return 0;
}

