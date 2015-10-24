#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

#define ARDUINO_POLL_TAG '0'
#define ARDUINO_READ_TAG '1'
#define ARDUINO_SET_TAG '2'

char command;
char lockerUid[32];
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
bool tagSetDown;

bool isTagPresent()
{
  // Check to see if our UUID NFC tag is present
  readCard();
  char tmpBuffer[32];
  memset(tmpBuffer, 0, 32);
  if ( uidLength == 4 )
    sprintf(tmpBuffer, "%02x:%02x:%02x:%02x", uid[0], uid[1], uid[2], uid[3]);
  else if ( uidLength == 7 )
    sprintf(tmpBuffer, "%02x:%02x:%02x:%02x:%02x:%02x:%02x", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
  if ( strcmp(lockerUid, tmpBuffer) == 0 )
    return true;
  return false;
}

bool readCard()
{
  uint8_t success;
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
//  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);   

  //// DEBUG BLOCK ////
  uidLength = 7;
  success = true;
  /////////////////////
  
  if ( success )
    return true;
  return false;
}

void setup() {
  Serial.begin(115200);
//  nfc.begin();
//  uint32_t versiondata = nfc.getFirmwareVersion();
//  if (! versiondata) {
//    while (1); // halt
//  }  
  // configure board to read RFID tags
//  nfc.SAMConfig();
  
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
      // Has the tag been set down?
      if ( tagSetDown == true )
      {
        // Check if tag is down now
        if ( isTagPresent() )
        {
          Serial.write("connected");
        }
        else
        {
          Serial.write("disconnected");
          tagSetDown = false;
        }
      }
      else
      {
        Serial.write("inactive");
        if ( isTagPresent() )
        {
          tagSetDown = true;
        }
      }
    }
    else if ( command == ARDUINO_READ_TAG )
    {
      if ( readCard() == true )
      {
        memset(lockerUid, 0, 32);
        if ( uidLength == 4 )
          sprintf(lockerUid, "%02x:%02x:%02x:%02x", uid[0], uid[1], uid[2], uid[3]);
        else if ( uidLength == 7 )
          sprintf(lockerUid, "%02x:%02x:%02x:%02x:%02x:%02x:%02x", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
      }
      Serial.write(lockerUid);
    }
    else if ( command == ARDUINO_SET_TAG )
    {
      char tmpBuf[32];
      String buf = Serial.readString();
      buf.toCharArray(tmpBuf, 32);
      strncpy(lockerUid, tmpBuf, 32);
    }
  }
}
