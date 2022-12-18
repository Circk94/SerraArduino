//www.ilsito.net
 
int luminosita; //Il valore letto dalla fotoresistenza
int valore = 900; //Soglia dopo la quale si accende il led
#define PIN_Fotoresistenza A1
 
void setup() {
  Serial.begin(9600);
}
 
void loop() {
  luminosita = analogRead(PIN_Fotoresistenza);  //Lettura della luminosità
  Serial.print("Luminosità: ");
  Serial.println(luminosita);
  delay(500);  //Aspetta mezzo secondo prima di ricontrollare
}