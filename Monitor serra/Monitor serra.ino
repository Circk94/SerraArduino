/*
  Test_lettura_sensore_LM33_new.ino
  Il programma misura la temperatura tramite il sensore LM35
  e mostra il valore sul display LCD

  Vengono utilizzati i seguenti pin
  Pin +5V         -> Alimentazione
  Pin GND         -> Alimentazione
  Pin Digital 4   -> Bus D4 dati LCD
  Pin Digital 5   -> Bus D5 dati LCD
  Pin Digital 6   -> Bus D6 dati LCD
  Pin Digital 7   -> Bus D7 dati LCD
  Pin Digital 8   -> terminale RS display LCD
  Pin Digital 9   -> terminale EN display LCD
  Pin Analogico 1 -> lettura LM35

  La scala dell'LM 35 è T = K * V
  dove:
  K = 100 [°C/V] coeff.angolare della scala
  V = tensione generata dall'LM35 (10mV/°C)
  T = temperatura in °C

  Include la libreria per la gestione del LCD

  Creato il 15/12/2011
  Aggiornato il 22/6/2019
  Sito web https://www.adrirobot.it
  Blog https://it.emcelettronica.com/author/adrirobot
  Pagina Facebook https://www.facebook.com/Adrirobot-318949048122955
  Istagram https://www.instagram.com/adrirobot/
  This example code is in the public domain.

*/

/* Numero del pin analogico sul quale è collegato l'LM35  */
#define LM35_pin 1

/* Definizioni globali */
float vref = 1.1;	                     // Vref dell'ADC (quell'interno è di 1,1V)

/* Impostazione dell'hardware */

void setup()
{
  analogReference( INTERNAL );  // per l'ADC usiamo il Vref interno da 1,1V (migliore precisione)
  analogRead( LM35_pin );       // Prima lettura "a vuoto" (serve per l'assestamento dell'ADC)
  Serial.begin(9600);
}

void loop()
{
  float temp = readTemp();  // legge il valore della temperatura e la memorizza nella var. temp.

  String tempString = String(temp, 3);

  Serial.print(tempString + "\r\n");
  
  delay(2000);
}

/* Legge la temperatura */
float readTemp()
{
  float temp = 0.0;       // valore convertito in temperatura (°C)
  int val = 0;            // valore quantizzato dall'ADC [0..1023]
  int nread = 5;          // numero di letture (da 5 a 8)
  float somma = 0.0;      // somma delle letture
  for (int i = 0; i < nread; i++)
  {
    val = analogRead( LM35_pin );              // legge il dato della tensione sul pin 'LM35_pin'
    temp = ( 100.0 *  vref * val ) / 1024.0;   // lo converte in °C
    somma += temp;                             // aggiunge alla somma delle temperature lette
  }
  return ( somma / nread );                     // ne calcola il valore medio
}