#include <EEPROM.h>

#include "DHT.h"
#include <LiquidCrystal_I2C.h>

/******************************************************************************/
/************************* SENSORI E ATTUATORI ********************************/
/******************************************************************************/
//Digital pin for humidity sensor
#define DHT1PIN 2     
#define DHT2PIN 3  

#define HEATERPIN 4
/* Definizioni globali */
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Initialize DHT sensor.
DHT dht1(DHT1PIN, DHTTYPE);
DHT dht2(DHT2PIN, DHTTYPE);
//Initialize LCD display
LiquidCrystal_I2C lcd(0x3F, 16, 2); // I2C address 0x3F, 16 column and 2 rows

//Mini JoyStick utilizzato per navigare tra i menù
const int matrixGrade = 4;
int pinUP = 5;
int pinDWN = 6;
int pinLFT = 7;
int pinRIGHT = 8;
int pinMID = 9;
const String cmdUP = "UP";
const String cmdDOWN = "DN";
const String cmdRIGHT = "R";
const String cmdLEFT = "L";
const String cmdOK = "OK";
const String cmdNoCommand = "NC";
/******************************************************************************/

/******************************************************************************/
/*********************************** TIMING ***********************************/
/******************************************************************************/
//Variabili per la gestione della temporizzazione
int loopDelay = 100; //valore in ms del delay nella funzione loop

const int measureDelay = 2000;  //valore in ms del delay tra due misure
int measureTiming = measureDelay;  //valore in ms del tempo trascorso dall'ultima misura (lo inizializzo a measureDelay per effettuare una misura già all'avvio)

const int readMatrixDisabledDelay = 300;
int readMatrixDisabledTiming = readMatrixDisabledDelay;
bool isCmdDetected = false;
/******************************************************************************/

/******************************************************************************/
/****************************** MISURE E SOGLIE *******************************/
/******************************************************************************/
#define EEPROMADDR_minTempL 0
#define EEPROMADDR_minTempT EEPROMADDR_minTempL + sizeof(float)
#define EEPROMADDR_maxTempL EEPROMADDR_minTempT + sizeof(float)
#define EEPROMADDR_maxTempT EEPROMADDR_maxTempL + sizeof(float)
float minTempL;
float minTempT;
float maxTempL;
float maxTempT;

float h = 0;
float t = 0;
/******************************************************************************/

/******************************************************************************/
/****************************** ON OFF MANUALE ********************************/
/******************************************************************************/
//Enumeratore coi possibili stati di accensione spegnimento
#define EEPROMADDR_lightOnOffState EEPROMADDR_maxTempT + sizeof(float)
#define EEPROMADDR_funROnOffState EEPROMADDR_lightOnOffState + sizeof(int)
#define EEPROMADDR_funLOnOffState EEPROMADDR_funROnOffState + sizeof(int)
String onOffStateLabel[] = {"AUTO", "ON  ", "OFF "};
// 0 = AUTO
// 1 = ON
// 2 = OFF
int lightOnOffState = 0;
int funROnOffState = 0;
int funLOnOffState = 0;
const int nPossibiliStati = 3;
/******************************************************************************/

/******************************************************************************/
/******************************** PAGINAZIONE *********************************/
/******************************************************************************/
//Indice del menu (incrementabile con destra e sinistra)
//0 = menu contenente le varie letture
//1 = menu per il settaggio delle soglie
int menu = 0;
int nMenu = 3;
const int idMenuLetture = 0;
const int idMenuOnOff = 1;
const int idMenuSoglie = 2;
const int pagineMenuLetture = 1;
const int pagineOnOffManuale = 2;
const int pagineMenuSettaggi = 2;
int paginePerMenu[] = {pagineMenuLetture, pagineOnOffManuale, pagineMenuSettaggi};
bool showCursorePerMenu[] = {false, true, true};
//Indice della pagina (incrementabile con sopra e sotto)
//0 = pagina principale
int pagina = 0;
//Indice della riga in cui si trova il cursore (che si vede nella prima cella 
//per i menù in cui è possibile selezionare qualcosa)
//0 = pagina principale
int cursore = 0;

//menù accensione spegnimento manuale
const String labelLedFullSpectrum = "light";
const String labelFunRight = "funR";
const String labelFunLeft = "funL";
String onOffPerPagina[pagineOnOffManuale][2] = {
  {labelFunRight, labelFunLeft},
  {labelLedFullSpectrum, ""}
};

//menu soglie
const String labelSoglia_minTempL = "minTempL";
const String labelSoglia_minTempT = "minTempT";
const String labelSoglia_maxTempL = "maxTempL";
const String labelSoglia_maxTempT = "maxTempT";
String sogliePerPagina[pagineMenuSettaggi][2] = {
  {labelSoglia_minTempL, labelSoglia_minTempT}, //Pagina 1
  {labelSoglia_maxTempL, labelSoglia_maxTempT}  //Pagina 2
};
bool editing = false;
const float deltaIncrDecrSoglia = 0.5f;

//Variabili per la gestione del cambio pagina
bool mpChanged = false; //controlla se il menù o la pagina sono state cambiate
/******************************************************************************/

/******************************************************************************/
/*********************************** VARIE ************************************/
/******************************************************************************/
//Byte array che definisce il carattere speciale per il cursore
byte pacmanBoccaChiusa[] = {
  0b01110,
  0b11101,
  0b11111,
  0b11111,
  0b11000,
  0b11111,
  0b11111,
  0b01110
};

byte pacmanBoccaAperta[] = {
  0b01110,
  0b11101,
  0b11111,
  0b11100,
  0b11000,
  0b11000,
  0b11111,
  0b01110
};

// byte alieno[] = {
//   0b10001,
//   0b01010,
//   0b11111,
//   0b10101,
//   0b11111,
//   0b11111,
//   0b01010,
//   0b11011
// };

/******************************************************************************/

//MAIN FUNCTIONS
void setup() {
  Serial.begin(9600);

  //Init sensore di umidità
  dht1.begin();
  dht2.begin();

  //Init LCD
  lcd.init(); // initialize the lcd
  lcd.backlight();

  //creo il nuovo carattere da usare come cursore
  lcd.createChar(0, pacmanBoccaAperta);
  lcd.createChar(1, pacmanBoccaChiusa);

  //Init Heater/Light
  pinMode(HEATERPIN, OUTPUT);           // set pin to input
  digitalWrite(HEATERPIN, LOW);

  //Inizializzo i pin digitali utilizzati per la lettura della matrice di pulsanti
  pinMode(pinUP, INPUT_PULLUP); 
  pinMode(pinDWN, INPUT_PULLUP); 
  pinMode(pinLFT, INPUT_PULLUP); 
  pinMode(pinRIGHT, INPUT_PULLUP); 
  pinMode(pinMID, INPUT_PULLUP); 

  //recupero i valori delle soglie salvati in eeprom
  EEPROM.get(EEPROMADDR_minTempL, minTempL);
  EEPROM.get(EEPROMADDR_minTempT, minTempT);
  EEPROM.get(EEPROMADDR_maxTempL, maxTempL);
  EEPROM.get(EEPROMADDR_maxTempT, maxTempT);

  //recupero i settaggi degli on off manuali salvati in eeprom
  EEPROM.get(EEPROMADDR_lightOnOffState, lightOnOffState);
  EEPROM.get(EEPROMADDR_funROnOffState, funROnOffState);
  EEPROM.get(EEPROMADDR_funLOnOffState, funLOnOffState);
}

void loop() {
  //delay tra due chiamate della funzione loop
  delay(loopDelay);
  //incremento le variabili temporali che tengono il tempo di ogni diversa operazione
  manageTiming();

  //Se il tempo tra due misure è trascorso
  if(measureTiming >= measureDelay){
    //azzero il contatore;
    measureTiming = 0;
    //effettuo le misure
    h = readHumid();
    t = readTemp();
    //gestisco i vari attuatori
    enableDisableHeater();
  }
  
  if(readMatrixDisabledTiming >= readMatrixDisabledDelay){
    String cmd = readJoyStick();
    if(cmd != cmdNoCommand){
      Serial.print("Azzero il disabled timing\n\r");
      readMatrixDisabledTiming = 0;
      if(!editing){
        manageMenuPageChange(cmd);
      }
      if(editing){
        manageEditing(cmd);
      }
      detectEditing(cmd);
    }
  }

  //il refresh dell'LDC funziona alla frequenza massima (del loop) per essere il più responsive possibile
  printToLCD();  
}

//FUNZIONI AUSILIARIE
//LETTURE
float readHumid(){
  float hToReturn = 0;
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h1 = dht1.readHumidity();
  float h2 = dht2.readHumidity(); 
  //Controllo eventuali errori di lettura
  bool dht1Fail = isnan(h1);
  bool dht2Fail = isnan(h2);
  if(dht1Fail && dht2Fail){
    Serial.print("Errore di lettura umidita");
  }
  if(!dht1Fail && dht2Fail){
    hToReturn = h1;
  }
  if(dht1Fail && !dht2Fail){
    hToReturn = h2;
  }
  if(!dht1Fail && !dht2Fail){
    hToReturn = (h1 + h2) / 2;
  }

  //LOG
  Serial.print(F("Humidity1: "));
  Serial.print(h1);
  Serial.print(F("Humidity2: "));
  Serial.print(h2);
  Serial.print(F("Humidity print: "));
  Serial.print(hToReturn);
  Serial.print(F("\n\r"));

  return hToReturn;
}

float readTemp(){
  float tToReturn = 0;
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float t1 = dht1.readTemperature();
  float t2 = dht2.readTemperature();
  //Controllo eventuali errori di lettura
  bool dht1Fail = isnan(t1);
  bool dht2Fail = isnan(t2);
  if(dht1Fail && dht2Fail){
    Serial.print("Errore di lettura temperatura");
  }
  if(!dht1Fail && dht2Fail){
    tToReturn = t1;
  }
  if(dht1Fail && !dht2Fail){
    tToReturn = t2;
  }
  if(!dht1Fail && !dht2Fail){
    tToReturn = (t1 + t2) / 2;
  }

  //LOG
  Serial.print(F("Temperature1: "));
  Serial.print(t1);
  Serial.print(F("°C "));
  Serial.print(F("Temperature2: "));
  Serial.print(t2);
  Serial.print(F("°C "));
  Serial.print(F("Temperature print: "));
  Serial.print(tToReturn);
  Serial.print(F("°C \n\r"));

  return tToReturn;
}

String readJoyStick(){
  //leggo tutti i pin digitali dai quali mi può arrivare un comando
  int tempCmdUP = digitalRead(pinUP);
  int tempCmdDOWN = digitalRead(pinDWN);
  int tempCmdLEFT = digitalRead(pinLFT);
  int tempCmdRIGHT = digitalRead(pinRIGHT);
  int tempCmdMID = digitalRead(pinMID);

  //Visto che la pressione centrale del tasto può mandare a 0 alcuni o tutti gli altri
  //segnali oltre al MID, per controllare se MID è stato premuto verifico anche
  //lo stato degli altri segnali
  if(tempCmdMID == 0){
    if(tempCmdUP == 0 && tempCmdDOWN == 0 && tempCmdLEFT == 0 && tempCmdRIGHT == 0){
      //Se sono tutti 0 sicuramente sto premendo il tasto centrale
      return cmdOK;
    }else if(tempCmdUP == 0 || tempCmdDOWN == 0 || tempCmdLEFT == 0 || tempCmdRIGHT == 0){
      //Se MID è 0 ed anche qualcun altro pin è 0, allora sto premendo il tasto 
      //centrale ma in maniera poco energica. Considero comunque come comando OK
      return cmdOK;
    }else{
      //Se MID è 0 e nessun altro è 0 considero NoCommand. Dai test non è stato osservato
      //che MID vada a 0 da solo. Pertanto viene considerato come un mal funzionamento (per ora).
      return cmdNoCommand;
    }
  }else{
    //Se MID è diverso da 0, controllo se sto eseguendo qualche altro comando
    if(tempCmdUP == 0){
      return cmdUP;
    }else if(tempCmdDOWN == 0){
      return cmdDOWN;
    }else if(tempCmdLEFT == 0){
      return cmdLEFT;
    }else if(tempCmdRIGHT == 0){
      return cmdRIGHT;
    }else{
      return cmdNoCommand;
    }
  }
}

//ATTUAZIONI
void enableDisableHeater(){  
  if(lightOnOffState == 0){
    if(t < minTempL){
      //se la temperatura è sotto la soglia minima, accendo l'heater
      digitalWrite(HEATERPIN, HIGH);
    }
    if(t > minTempT){
      //se la temperatura è sopra la soglia massima, spengo l'heater
      digitalWrite(HEATERPIN, LOW);
    }
  }else if(lightOnOffState == 1){
    digitalWrite(HEATERPIN, HIGH);
  }else if(lightOnOffState == 2){
    digitalWrite(HEATERPIN, LOW);
  }
}

void printToLCD(){
  //Solo se il menù o la pagina sono stati cambiati
  if(mpChanged){
    //pulisco lo schermo
    lcd.clear();
    mpChanged = false;
  }

  //Se il menù selezionato prevede il cursore lo scrivo
  if(showCursorePerMenu[menu]){
    lcd.setCursor(0, cursore);
    if(editing){
      lcd.write(byte(1));
    }else{
      lcd.write(byte(0));
    }
    if(cursore == 0){
      lcd.setCursor(0, cursore + 1);
      lcd.print(" ");
    }else if(cursore == 1){
      lcd.setCursor(0, cursore - 1);
      lcd.print(" ");
    }    
  }
  
  switch (menu) {
    case 0:
      printMenu0();
      break;
    case 1: 
      printMenu1();
      break;
    case 2:
      printMenu2();
      break;
    // case 3:
    //   lcd.setCursor(1, 0);
    //   lcd.print("Quarto menu");
    //   break;
    default:
      break;
  }
}

//Menù di visualizzazione delle misure
void printMenu0(){
  //Menù 0: visualizzazione delle misure
  //        0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
  //riga0:    t e m p : x x . x x  °  C     L  o
  //riga1:    u m i d : x x . x x  %
  if(pagina == 0){
    //Intestazione righe indicazione della grandezza rappresentata
    lcd.setCursor(1, 0);        
    lcd.print("temp:");         
    lcd.setCursor(1, 1);        
    lcd.print("umid:");          
    //Valori misurati
    lcd.setCursor(6, 0);
    lcd.print(t); 
    lcd.setCursor(6, 1);
    lcd.print(h);
    //Unità di misura 
    lcd.setCursor(11, 0);
    lcd.print((char)223);
    lcd.setCursor(12, 0);
    lcd.print("C");
    lcd.setCursor(11, 1);  
    lcd.print("%");
    //Indicatori violazione soglia
    //Temperatura
    if(t < minTempL){
      lcd.setCursor(14, 0);
      lcd.print("Lo");
    }
    if(t > maxTempT){
      lcd.setCursor(14, 0);
      lcd.print("Hi");
    }
    if(t >= minTempL && t <= maxTempT){
      lcd.setCursor(14, 0);
      lcd.print("  ");
    }
  }else if(pagina == 1){
    lcd.setCursor(1, 0);        
    lcd.print("Menu1,pagina2");
  }else if(pagina == 2){
    lcd.setCursor(1, 0);        
    lcd.print("Menu1,pagina3");
  }  
}

//Menù accensione spegnimento manuali
void printMenu1(){
  //Menù 2: accensione spegnimento manuale
  //        0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
  //riga0:  > f u n R   A U T O
  //riga1:    f u n L   A U T O
  lcd.setCursor(1, 0); 
  lcd.print(onOffPerPagina[pagina][0]);
  lcd.setCursor(1, 1); 
  lcd.print(onOffPerPagina[pagina][1]);
  if(pagina == 0){
    lcd.setCursor(onOffPerPagina[pagina][0].length() + 2, 0);
    lcd.print(onOffStateLabel[funROnOffState]);
    lcd.setCursor(onOffPerPagina[pagina][1].length() + 2, 1);        
    lcd.print(onOffStateLabel[funLOnOffState]);
  }else if(pagina == 1){
    lcd.setCursor(onOffPerPagina[pagina][0].length() + 2, 0);   
    lcd.print(onOffStateLabel[lightOnOffState]);
    lcd.setCursor(onOffPerPagina[pagina][1].length() + 2, 1);        
    lcd.print("");
  }
}

//Menù per i settaggi delle soglie
void printMenu2(){
  //Menù 1: settaggio soglie
  //        0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
  //riga0:  > m i n T e m p L : x  x  .  x  x 
  //riga1:    m i n T e m p T : x  x  .  x  x 
  lcd.setCursor(1, 0); 
  lcd.print(sogliePerPagina[pagina][0]);
  lcd.setCursor(1, 1); 
  lcd.print(sogliePerPagina[pagina][1]);
  if(pagina == 0){
    lcd.setCursor(10, 0);        
    lcd.print(minTempL);
    lcd.setCursor(10, 1);        
    lcd.print(minTempT);
  }else if(pagina == 1){
    lcd.setCursor(10, 0);        
    lcd.print(maxTempL);
    lcd.setCursor(10, 1);        
    lcd.print(maxTempT);
  }
}

void manageMenuPageChange(String cmd){
  if(cmd == cmdRIGHT || cmd == cmdLEFT){
    moveMenu(cmd);
    mpChanged = true;
  }
  if(cmd == cmdUP || cmd == cmdDOWN){
    //se il cursore è presente devo muoverlo in alto o in basso
    bool changePage = false;
    if(showCursorePerMenu[menu]){
      changePage = moveCursor(cmd);
    }else{
      changePage = true;
    }
    //Decido se cambiare pagina in base allo spostamento del cursore
    //ed in base al numero di pagine definito per quel menù
    if(changePage){
      movePage(cmd);
      changePage = false;
    }
  }
}

void detectEditing(String cmd){
  if(cmd == cmdOK && showCursorePerMenu[menu]){
    editing = !editing;
    if(menu == idMenuOnOff){
      if(editing){
        if(onOffPerPagina[pagina][cursore] == labelLedFullSpectrum){
          EEPROM.get(EEPROMADDR_lightOnOffState, lightOnOffState);
        }else if(onOffPerPagina[pagina][cursore] == labelFunRight){
          EEPROM.get(EEPROMADDR_funROnOffState, funROnOffState);
        }else if(onOffPerPagina[pagina][cursore] == labelFunLeft){
          EEPROM.get(EEPROMADDR_funLOnOffState, funLOnOffState);
        }
      }else{
        if(onOffPerPagina[pagina][cursore] == labelLedFullSpectrum){
          EEPROM.put(EEPROMADDR_lightOnOffState, lightOnOffState);
        }else if(onOffPerPagina[pagina][cursore] == labelFunRight){
          EEPROM.put(EEPROMADDR_funROnOffState, funROnOffState);
        }else if(onOffPerPagina[pagina][cursore] == labelFunLeft){
          EEPROM.put(EEPROMADDR_funLOnOffState, funLOnOffState);
        }
      }
    }else if(menu == idMenuSoglie){
      if(editing){
        //ho abilitato l'editing delle soglie, quindi recupero dalla EEPROM
        //il valore salvato e lo salvo nella variabile locale 
        //per essere editato
        if(sogliePerPagina[pagina][cursore] == labelSoglia_minTempL){
          EEPROM.get(EEPROMADDR_minTempL, minTempL);
        }else if(sogliePerPagina[pagina][cursore] == labelSoglia_minTempT){
          EEPROM.get(EEPROMADDR_minTempT, minTempT);
        }else if(sogliePerPagina[pagina][cursore] == labelSoglia_maxTempL){
          EEPROM.get(EEPROMADDR_maxTempL, maxTempL);
        }else if(sogliePerPagina[pagina][cursore] == labelSoglia_maxTempT){
          EEPROM.get(EEPROMADDR_maxTempT, maxTempT);
        }
      }else{
        //ho disabilitato l'editing delle soglie, quindi salvo il valore
        //della variabile locale in EEPROM
        if(sogliePerPagina[pagina][cursore] == labelSoglia_minTempL){
          EEPROM.put(EEPROMADDR_minTempL, minTempL);
        }else if(sogliePerPagina[pagina][cursore] == labelSoglia_minTempT){
          EEPROM.put(EEPROMADDR_minTempT, minTempT);
        }else if(sogliePerPagina[pagina][cursore] == labelSoglia_maxTempL){
          EEPROM.put(EEPROMADDR_maxTempL, maxTempL);
        }else if(sogliePerPagina[pagina][cursore] == labelSoglia_maxTempT){
          EEPROM.put(EEPROMADDR_maxTempT, maxTempT);
        }
      }
    }
  }
}

void manageEditing(String cmd){
  if(menu == idMenuOnOff){
    editOnOff(cmd);
  }else if(menu == idMenuSoglie){
    editSoglie(cmd);
  }
}

void editOnOff(String cmd){  
  if(onOffPerPagina[pagina][cursore] == labelLedFullSpectrum){
    if(cmd == cmdDOWN){
      if(lightOnOffState < (nPossibiliStati - 1)){
        lightOnOffState++;
      }else if(lightOnOffState == (nPossibiliStati - 1)){
        lightOnOffState = 0;
      }
    }
    if(cmd == cmdUP){
      if(lightOnOffState > 0){
        lightOnOffState--;
      }else if(lightOnOffState == 0){
        lightOnOffState = nPossibiliStati - 1;
      }
    }
  }else if(onOffPerPagina[pagina][cursore] == labelFunRight){
    if(cmd == cmdDOWN){
      if(funROnOffState < (nPossibiliStati - 1)){
        funROnOffState++;
      }else if(funROnOffState == (nPossibiliStati - 1)){
        funROnOffState = 0;
      }
    }
    if(cmd == cmdUP){
      if(funROnOffState > 0){
        funROnOffState--;
      }else if(funROnOffState == 0){
        funROnOffState = nPossibiliStati - 1;
      }
    }
  }else if(onOffPerPagina[pagina][cursore] == labelFunLeft){
    if(cmd == cmdDOWN){
      if(funLOnOffState < (nPossibiliStati - 1)){
        funLOnOffState++;
      }else if(funLOnOffState == (nPossibiliStati - 1)){
        funLOnOffState = 0;
      }
    }
    if(cmd == cmdUP){
      if(funLOnOffState > 0){
        funLOnOffState--;
      }else if(funLOnOffState == 0){
        funLOnOffState = nPossibiliStati - 1;
      }
    }
  }
}

void editSoglie(String cmd){  
  float valoreDaSommare = 0.00f;
  if(cmd == cmdUP){
    //Sto incrementando il valore
    valoreDaSommare += deltaIncrDecrSoglia;
  }else if(cmd == cmdDOWN){
    //Sto decrementando il valore
    valoreDaSommare -= deltaIncrDecrSoglia;
  }
  if(sogliePerPagina[pagina][cursore] == labelSoglia_minTempL){
    minTempL += valoreDaSommare;
  }else if(sogliePerPagina[pagina][cursore] == labelSoglia_minTempT){
    minTempT += valoreDaSommare;
  }else if(sogliePerPagina[pagina][cursore] == labelSoglia_maxTempL){
    maxTempL += valoreDaSommare;
  }else if(sogliePerPagina[pagina][cursore] == labelSoglia_maxTempT){
    maxTempT += valoreDaSommare;
  }
}

//Sposta il cursore in alto o un basso e restituisce un bool
//che indica se bisogna anche cambiare pagina
bool moveCursor(String upDown){
  //LCD 
  //cursore 0 ***************
  //cursore 1 ***************
  bool changePage = false;
  if(upDown == cmdUP){
    if(cursore == 0){
      //se ho premuto UP e il cursore è già a 0 lo porto a 1
      cursore = 1;
      //e devo cambiare pagina (andando a quella precedente)
      changePage = true;
    }else if(cursore == 1){
      //se il cursore era a 1, lo mando a 0 ma non cambio pagina
      cursore = 0;
    }
  }else if(upDown == cmdDOWN){
    if(cursore == 0){
      //se ho premuto DOWN e il cursore è a 0 lo porto a 1 ma non cambio pagina
      cursore = 1;
    }else if(cursore == 1){
      //se il cursore era a 1, lo mando a 0
      cursore = 0;
      //e devo cambiare pagina (andando a quella successiva)
      changePage = true;
    }
  }
  return changePage;
}

void movePage(String cmd){
  //recupero il numero di pagine disponibili per quel menù
  int previousPage = pagina;
  int maxPagina = paginePerMenu[menu];
  if(cmd == cmdDOWN){
    if(pagina < (maxPagina - 1)){
      pagina++;
    }else if(pagina == (maxPagina - 1)){
      pagina = 0;
    }
  }
  if(cmd == cmdUP){
    if(pagina > 0){
      pagina--;
    }else if(pagina == 0){
      pagina = maxPagina - 1; //perchè l'indice di pagina è zero based, ma il numero di pagine parte da 1
    }
  }
  if(previousPage != pagina){
    mpChanged = true;
  }
}

void moveMenu(String cmd){
  if(cmd == cmdRIGHT){
    if(menu < (nMenu - 1)){
      menu++;
    }else if(menu == (nMenu - 1)){
      menu = 0;
    }
  }
  if(cmd == cmdLEFT){
    if(menu > 0){
      menu--;
    }else if(menu == 0){
      menu = nMenu - 1;
    }
  }
  //Se sto cambiando menù azzero l'indice delle pagine e del cursore
  pagina = 0;
  cursore = 0;
}

void manageTiming(){
  measureTiming += loopDelay;
  if(readMatrixDisabledTiming < readMatrixDisabledDelay){
    readMatrixDisabledTiming += loopDelay;
  }
}