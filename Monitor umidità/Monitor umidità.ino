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

//matrice di pulsanti usata come tastierino
const int matrixGrade = 4;
int pinRighe[matrixGrade] = {12, 11, 10, 9};
int pinColonne[matrixGrade] = {8, 7, 6, 5};
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
/******************************** PAGINAZIONE *********************************/
/******************************************************************************/
//Indice del menu (incrementabile con destra e sinistra)
//0 = menu contenente le varie letture
//1 = menu per il settaggio delle soglie
int menu = 0;
int nMenu = 2;
const int pagineMenuLetture = 1;
const int pagineMenuSettaggi = 2;
int paginePerMenu[] = {pagineMenuLetture, pagineMenuSettaggi};
bool showCursorePerMenu[] = {false, true};
//Indice della pagina (incrementabile con sopra e sotto)
//0 = pagina principale
int pagina = 0;
//Indice della riga in cui si trova il cursore (che si vede nella prima cella 
//per i menù in cui è possibile selezionare qualcosa)
//0 = pagina principale
int cursore = 0;

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
  for(int i = 0; i < matrixGrade; i++){
    pinMode(pinColonne[i], OUTPUT);
  }
  for(int i = 0; i < matrixGrade; i++){
    pinMode(pinRighe[i], INPUT);
  }

  //recupero i valori delle soglie salvati in eeprom
  EEPROM.get(EEPROMADDR_minTempL, minTempL);
  EEPROM.get(EEPROMADDR_minTempT, minTempT);
  EEPROM.get(EEPROMADDR_maxTempL, maxTempL);
  EEPROM.get(EEPROMADDR_maxTempT, maxTempT);
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
    String cmd = readTastierino();
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

String readTastierino(){
  /*******************************/
  /*** nu *** nu *** nu *** nu ***/
  /*******************************/
  /*** nu *** nu *** UP *** nu ***/
  /*******************************/
  /*** nu *** SX *** OK *** DS ***/
  /*******************************/
  /*** nu *** nu *** DN *** nu ***/
  /*******************************/
  //NB: Il pulsante del 3 è rotto!

  //Metto preventivamente tutti i pin delle colonne a LOW
  for(int i = 0; i < matrixGrade; i++){
    digitalWrite(pinColonne[i], LOW);
  }

  //Ciclo le colonne e le metto HIGH una alla volta
  for(int i = 0; i < matrixGrade; i++){
    digitalWrite(pinColonne[i], HIGH);

    if(digitalRead(pinRighe[0]) == HIGH && digitalRead(pinRighe[1]) == LOW && digitalRead(pinRighe[2]) == LOW && digitalRead(pinRighe[3]) == LOW){
      switch(i){
        case 0:
          Serial.print("1");
          return cmdNoCommand;
        case 1:
          Serial.print("4");
          return cmdNoCommand;
        case 2:
          Serial.print("7");
          return cmdNoCommand;
        case 3:
          Serial.print("*");
          return cmdNoCommand;
        default:
          return cmdNoCommand;
      }
    }else if(digitalRead(pinRighe[0]) == LOW && digitalRead(pinRighe[1]) == HIGH && digitalRead(pinRighe[2]) == LOW && digitalRead(pinRighe[3]) == LOW){
      switch(i){
        case 0:
          Serial.print("2");
          return cmdNoCommand;
        case 1:
          Serial.print("5");
          return cmdNoCommand;
        case 2:
          Serial.print("Comando tastierino: " + cmdLEFT);
          return cmdLEFT;
        case 3:
          Serial.print("0");
          return cmdNoCommand;
        default:
          return cmdNoCommand;
      }
    }else if(digitalRead(pinRighe[0]) == LOW && digitalRead(pinRighe[1]) == LOW && digitalRead(pinRighe[2]) == HIGH && digitalRead(pinRighe[3]) == LOW){
      switch(i){
        case 0:
          Serial.print("3");
          return cmdNoCommand;
        case 1:
          Serial.print("Comando tastierino: " + cmdUP);
          return cmdUP;
        case 2:
          Serial.print("Comando tastierino: " + cmdOK);
          return cmdOK;
        case 3:
          Serial.print("Comando tastierino: " + cmdDOWN);
          return cmdDOWN;
        default:
          return cmdNoCommand;
      }
    }else if(digitalRead(pinRighe[0]) == LOW && digitalRead(pinRighe[1]) == LOW && digitalRead(pinRighe[2]) == LOW && digitalRead(pinRighe[3]) == HIGH){
      switch(i){
        case 0:
          Serial.print("A");
          return cmdNoCommand;
        case 1:
          Serial.print("B");
          return cmdNoCommand;
        case 2:
          Serial.print("Comando tastierino: " + cmdRIGHT);
          return cmdRIGHT;
        case 3:
          Serial.print("D");
          return cmdNoCommand;
        default:
          return cmdNoCommand;
      }
    }

    digitalWrite(pinColonne[i], LOW);
  }

  return cmdNoCommand;
}

//ATTUAZIONI
void enableDisableHeater(){  
  if(t < minTempL){
    //se la temperatura è sotto la soglia minima, accendo l'heater
    digitalWrite(HEATERPIN, HIGH);
  }
  if(t > minTempT){
    //se la temperatura è sopra la soglia massima, spengo l'heater
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
    // case 2:
    //   lcd.setCursor(1, 0);
    //   lcd.print("Terzo menu");
    //   break;
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

//Menù per i settaggi delle soglie
void printMenu1(){
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
    if(menu == 1){
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
  if(menu == 1){
    editSoglie(cmd);
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