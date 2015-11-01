#include <PN532.h>

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (13)
#define PN532_MOSI (11)
#define PN532_SS   (10)
#define PN532_MISO (12)

PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

#define ARDUINO_POLL_TAG '0'
#define ARDUINO_READ_TAG '1'
#define ARDUINO_SET_TAG '2'

char command;
int lockerUid;
int readUid;
bool tagSetDown;

bool isTagPresent()
{
  // Check to see if our UUID NFC tag is present
  readCard();

  char buf[128];
  if ( readUid == lockerUid )
  {
    return true;
  }
  return false;
}

void readCard()
{   
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  readUid = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A);
  if ( !readUid )
    readUid = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A); // bug in firmware yay
}

void setup() {
  Serial.begin(115200);
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    while (1); // halt
  }  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  tagSetDown = false;
}

void loop() {
  // Check to see if the rfid unit has been placed
  
  // put your main code here, to run repeatedly:
  if ( Serial.available() > 0 )
  {
    command = Serial.read(); // read a byte
    if ( command == ARDUINO_POLL_TAG )
    {
      if ( isTagPresent() == true )
      {
        Serial.write("1");
      }
      else
      {
        Serial.write("0");
      }
    }
    else if ( command == ARDUINO_READ_TAG )
    {
      readCard();
      if ( readUid )
      {
        char tmpBuf[32];
        sprintf(tmpBuf, "%i", readUid);
        Serial.write(tmpBuf);
      }
      else
      {
        Serial.write("-");
      }
    }
    else if ( command == ARDUINO_SET_TAG )
    {
      String buf = Serial.readString();
      if ( buf.length() > 0 )
      {
        lockerUid = buf.toInt();
        Serial.write("1");
      } else
      {
        Serial.write("0");
      }
    }
  }
}
