#define IGROMETROPIN A0
#define ENABLE_IGROMETRO 10

const int MIN_ADCREAD = 5;  //valore di massima umidità misurato con igrometro immerso in acqua
const int MAX_ADCREAD = 1023; //valore di minima umidità misurato con igrometro in aria

void setup() {
  Serial.begin(9600);
  pinMode(ENABLE_IGROMETRO, OUTPUT);
  digitalWrite(ENABLE_IGROMETRO, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  int lettura = LetturaIgrometro();

  Serial.println("Lettura igrometro:" + String(lettura));

  delay(1000);
}

//restituisce il valore percentuale dell'umidità del terreno misurata
int LetturaIgrometro(){
  //Alimento l'igrometro solo quando serve in modo da ridurre il deterioramento
  //della sonda
  digitalWrite(ENABLE_IGROMETRO, HIGH);
  delay(500);
  int ADCValue = analogRead(IGROMETROPIN);
  digitalWrite(ENABLE_IGROMETRO, LOW);

  //Mappo il valore letto con i valori minimo e massimo per convertirlo in percentuale
  int hPercValue = map(ADCValue, MIN_ADCREAD, MAX_ADCREAD, 100, 0);
  hPercValue = constrain(hPercValue, 0, 100);

  return ADCValue;
}

//TEST Taratura igrometro terreno
//Letture analogiche (media a regime):
//Igrometro in aria: 1023
//Igrometro in acqua: 108