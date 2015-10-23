// RfidLocker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "RfidLocker.h"
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

   HWND hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN), 0, DialogProc, 0);

   //HWND hWnd = CreateWindowEx(NULL, szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
   //	CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, hInstance, nullptr);

   if (!hDlg)
   {
      return FALSE;
   }

   // If this is the first start, show the window, otherwise hide on boot
 

   //ShowWindow(hDlg, nCmdShow);
   ShowWindow(hDlg, SW_HIDE);
   UpdateWindow(hDlg);
   AddTrayIcon(hDlg);

   return TRUE;
}

// Message handler for about box.
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
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
		PostQuitMessage(0);
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDM_EXIT)
		{
			DestroyWindow(hDlg);
			return TRUE; /* just continue reading on... */
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
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
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
