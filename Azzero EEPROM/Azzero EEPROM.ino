#include <EEPROM.h>

#define EEPROMADDR_minTempL 0
#define EEPROMADDR_minTempT EEPROMADDR_minTempL + sizeof(float)
#define EEPROMADDR_maxTempL EEPROMADDR_minTempT + sizeof(float)
#define EEPROMADDR_maxTempT EEPROMADDR_maxTempL + sizeof(float)
#define EEPROMADDR_minHSoil EEPROMADDR_maxTempT + sizeof(float)
#define EEPROMADDR_maxHSoil EEPROMADDR_minHSoil + sizeof(float)
#define EEPROMADDR_dayNight EEPROMADDR_maxHSoil + sizeof(float)
float minTempL;
float minTempT;
float maxTempL;
float maxTempT;
float minHSoil;
float maxHSoil;
int dayNight;

#define EEPROMADDR_lightOnOffState EEPROMADDR_dayNight + sizeof(int)
#define EEPROMADDR_funROnOffState EEPROMADDR_lightOnOffState + sizeof(int)
#define EEPROMADDR_funLOnOffState EEPROMADDR_funROnOffState + sizeof(int)
#define EEPROMADDR_waterOnOffState EEPROMADDR_funLOnOffState + sizeof(int)
int lightOnOffState = 0;
int funROnOffState = 0;
int funLOnOffState = 0;
int waterOnOffState = 0;

void setup() {
  EEPROM.put(EEPROMADDR_minTempL, 22.00f);
  EEPROM.put(EEPROMADDR_minTempT, 24.00f);
  EEPROM.put(EEPROMADDR_maxTempL, 26.00f);
  EEPROM.put(EEPROMADDR_maxTempT, 28.00f);
  EEPROM.put(EEPROMADDR_minHSoil, 80.00f);
  EEPROM.put(EEPROMADDR_maxHSoil, 95.00f);
  EEPROM.put(EEPROMADDR_dayNight, 450);
  EEPROM.put(EEPROMADDR_lightOnOffState, 0);
  EEPROM.put(EEPROMADDR_funROnOffState, 0);
  EEPROM.put(EEPROMADDR_funLOnOffState, 0);
  EEPROM.put(EEPROMADDR_waterOnOffState, 0);
  
  EEPROM.get(EEPROMADDR_minTempL, minTempL);
  EEPROM.get(EEPROMADDR_minTempT, minTempT);
  EEPROM.get(EEPROMADDR_maxTempL, maxTempL);
  EEPROM.get(EEPROMADDR_maxTempT, maxTempT);
  EEPROM.get(EEPROMADDR_minHSoil, minHSoil);
  EEPROM.get(EEPROMADDR_maxHSoil, maxHSoil);
  EEPROM.get(EEPROMADDR_dayNight, dayNight);
  EEPROM.get(EEPROMADDR_lightOnOffState, lightOnOffState);
  EEPROM.get(EEPROMADDR_funROnOffState, funROnOffState);
  EEPROM.get(EEPROMADDR_funLOnOffState, funLOnOffState);
  EEPROM.get(EEPROMADDR_waterOnOffState, waterOnOffState);

  Serial.begin(9600);
  delay(5000);
  // EEPROM.get(EEPROMADDR_minTempL, minTempL);
  // EEPROM.get(EEPROMADDR_minTempT, minTempT);
  // EEPROM.get(EEPROMADDR_maxTempL, maxTempL);
  // EEPROM.get(EEPROMADDR_maxTempT, maxTempT);
  // Serial.begin(9600);
  // delay(5000);
  Serial.println(minTempL);
  Serial.println(minTempT);
  Serial.println(maxTempL);
  Serial.println(maxTempT);
  Serial.println(minHSoil);
  Serial.println(maxHSoil);
  Serial.println(dayNight);
  Serial.println(lightOnOffState);
  Serial.println(funROnOffState);
  Serial.println(funLOnOffState);
  Serial.println(waterOnOffState);
  // if(minTempL != 0.00f){
  //   Serial.print("Azzero eeprom all'indirizzo di minTempL");
  //   Serial.print("\n\r");
  //   EEPROM.put(EEPROMADDR_minTempL, 0.00f);
  // }
  // if(minTempT != 0.00f){
  //   Serial.print("Azzero eeprom all'indirizzo di minTempT");
  //   Serial.print("\n\r");
  //   EEPROM.put(EEPROMADDR_minTempT, 0.00f);
  // }
  // if(maxTempL != 0.00f){
  //   Serial.print("Azzero eeprom all'indirizzo di maxTempL");
  //   Serial.print("\n\r");
  //   EEPROM.put(EEPROMADDR_maxTempL, 0.00f);
  // }
  // if(maxTempT != 0.00f){
  //   Serial.print("Azzero eeprom all'indirizzo di maxTempT");
  //   Serial.print("\n\r");
  //   EEPROM.put(EEPROMADDR_maxTempT, 0.00f);
  // }
}

void loop() {
  // put your main code here, to run repeatedly:

}
