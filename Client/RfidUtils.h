#pragma once

// Local includes
#include "SimpleIni.h"
#include "SerialClass.h"

#define ARDUINO_POLL_TAG "0"
#define ARDUINO_READ_TAG "1"
#define ARDUINO_SET_TAG "2"

#define CARD_NOT_PRESENT "0"
#define CARD_PRESENT "1"

void InitRfidLocker();
void InitArduino();
void ChangeCOMPort(int);
void ReadNewTag(char *);
void SetFrequency(int);
void CheckCurrentTag();
void SaveRfidSettings();
char * GetTag();

struct rfidOptions {
	char readTag[2048];
	long comNumber;
	long frequency;
};

// Thread friendly
extern CSimpleIniA g_ini;
extern struct rfidOptions g_rfidOptions;
extern Serial * g_SP;