#pragma once

// Local includes
#include "SimpleIni.h"
#include "SerialClass.h"

#define ARDUINO_POLL "0"
#define ARDUINO_UUID "1"

void InitRfidLocker();
void InitArduino();
void ChangeCOMPort(int);
void ReadNewTag(char *);
void SetFrequency(int);
void CheckCurrentTag();
void SaveRfidSettings();

struct rfidOptions {
	char readTag[2048];
	long comNumber;
	long frequency;
};

// Thread friendly
extern CSimpleIniA g_ini;
extern struct rfidOptions g_rfidOptions;
extern Serial * g_SP;