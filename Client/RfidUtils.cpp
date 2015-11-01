#include "stdafx.h"
#include "RfidUtils.h"

CSimpleIniA g_ini;
struct rfidOptions g_rfidOptions;
Serial * g_SP;

void InitRfidLocker()
{
	TCHAR path[MAX_PATH];
	SHGetSpecialFolderPath(0, path, CSIDL_APPDATA, FALSE);
	wcscat(path, L"\\RfidLocker");
	CreateDirectory(path, NULL);
	wcscat(path, L"\\RfidLocker.ini");
	g_ini.LoadFile(path);
	const char * readTag = g_ini.GetValue("settings", "readtag", "");
	strncpy(g_rfidOptions.readTag, readTag, 2048);
	g_rfidOptions.comNumber = g_ini.GetLongValue("settings", "com", 0);
	g_rfidOptions.frequency = g_ini.GetLongValue("settings", "frequency", 1);
	SaveRfidSettings();
}

void InitArduino()
{
	g_SP = NULL;
	ChangeCOMPort(g_rfidOptions.comNumber);
}

void ChangeCOMPort(int com)
{
	TCHAR comNum[16];
	wsprintf(comNum, L"COM%d", com);

	TCHAR comPort[1024];
	wcscpy(comPort, L"\\\\.\\");
	wcscat(comPort, comNum);
	if (g_SP != NULL)
	{
		delete g_SP;
	}
	g_SP = new Serial(comPort);    // adjust as needed

	g_rfidOptions.comNumber = com;

	// Send Tag ID
	if (g_SP->IsConnected() == true)
	{
		g_SP->WriteData(ARDUINO_SET_TAG, 1);
		Sleep(50);
		g_SP->WriteData(g_rfidOptions.readTag, strlen(g_rfidOptions.readTag));
	}
}

void SetFrequency(int frequency)
{
	g_rfidOptions.frequency = frequency;
}

void SaveRfidSettings()
{
	g_ini.SetValue("settings", "readtag", g_rfidOptions.readTag);
	g_ini.SetLongValue("settings", "com", g_rfidOptions.comNumber);
	g_ini.SetLongValue("settings", "frequency", g_rfidOptions.frequency);
	TCHAR path[MAX_PATH];
	SHGetSpecialFolderPath(0, path, CSIDL_APPDATA, FALSE);
	wcscat(path, L"\\RfidLocker\\RfidLocker.ini");
	g_ini.SaveFile(path);
}

char * GetTag()
{
	return g_rfidOptions.readTag;
}

void ReadNewTag(char * buffer)
{
	strncpy(g_rfidOptions.readTag, buffer, 2048);
}

void CheckCurrentTag()
{

}
