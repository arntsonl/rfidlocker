#define ARDUINO_POLL '0'
#define ARDUINO_UUID '1'

char command;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // Check to see if the rfid unit has been placed
  
  // put your main code here, to run repeatedly:
  if ( Serial.available() > 0 )
  {
    command = Serial.read(); // read a byte
    if ( command == ARDUINO_POLL )
    {
      Serial.write("connected");
    }
    else if ( command == ARDUINO_UUID )
    {
      Serial.write("0000000-0000-0000-000000000000");
    }
  }
}
