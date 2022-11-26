#include <EEPROM.h>

#define EEPROMADDR_minTempL 0
#define EEPROMADDR_minTempT EEPROMADDR_minTempL + sizeof(float)
#define EEPROMADDR_maxTempL EEPROMADDR_minTempT + sizeof(float)
#define EEPROMADDR_maxTempT EEPROMADDR_maxTempL + sizeof(float)
float minTempL;
float minTempT;
float maxTempL;
float maxTempT;

void setup() {
  // put your setup code here, to run once:
  EEPROM.get(EEPROMADDR_minTempL, minTempL);
  EEPROM.get(EEPROMADDR_minTempT, minTempT);
  EEPROM.get(EEPROMADDR_maxTempL, maxTempL);
  EEPROM.get(EEPROMADDR_maxTempT, maxTempT);
  Serial.begin(9600);
  delay(5000);
  Serial.print(minTempL);
  Serial.print("\n\r");
  Serial.print(minTempT);
  Serial.print("\n\r");
  Serial.print(maxTempL);
  Serial.print("\n\r");
  Serial.print(maxTempT);
  Serial.print("\n\r");
  if(minTempL != 0.00f){
    Serial.print("Azzero eeprom all'indirizzo di minTempL");
    Serial.print("\n\r");
    EEPROM.put(EEPROMADDR_minTempL, 0.00f);
  }
  if(minTempT != 0.00f){
    Serial.print("Azzero eeprom all'indirizzo di minTempT");
    Serial.print("\n\r");
    EEPROM.put(EEPROMADDR_minTempT, 0.00f);
  }
  if(maxTempL != 0.00f){
    Serial.print("Azzero eeprom all'indirizzo di maxTempL");
    Serial.print("\n\r");
    EEPROM.put(EEPROMADDR_maxTempL, 0.00f);
  }
  if(maxTempT != 0.00f){
    Serial.print("Azzero eeprom all'indirizzo di maxTempT");
    Serial.print("\n\r");
    EEPROM.put(EEPROMADDR_maxTempT, 0.00f);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
