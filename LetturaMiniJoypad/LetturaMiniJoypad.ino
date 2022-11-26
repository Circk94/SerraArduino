int UP = 8;
int DWN = 9;
int LFT = 10;
int RHT = 11;
int MID = 12;

void setup() {
  Serial.begin(9600);   
  
  pinMode(UP, INPUT_PULLUP); 
  pinMode(DWN, INPUT_PULLUP); 
  pinMode(LFT, INPUT_PULLUP); 
  pinMode(RHT, INPUT_PULLUP); 
  pinMode(MID, INPUT_PULLUP); 
  
}

void loop() {
  int cmdUP = digitalRead(UP);
  int cmdDWN = digitalRead(DWN);
  int cmdLEFT = digitalRead(LFT);
  int cmdRHT = digitalRead(RHT);
  int cmdMID = digitalRead(MID);
  
  Serial.print("UP: ");
  Serial.print(cmdUP);
  Serial.print(" | DWN: ");
  Serial.print(cmdDWN);
  Serial.print(" | LFT: ");
  Serial.println(cmdLEFT);
  Serial.print(" | RHT: ");
  Serial.println(cmdRHT);
  Serial.print(" | MID: ");
  Serial.println(cmdMID);

  delay(200);
  
}