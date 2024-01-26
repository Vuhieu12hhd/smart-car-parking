#include <EEPROM.h>
int address = 0;

int value = 4;
void setup() {

  Serial.begin(115200);
}

void loop() {
  
  //value = EEPROM.read(address);
  EEPROM.write(address,value);
  Serial.print("Value: ");
  Serial.println(value);
  delay(100);
}
