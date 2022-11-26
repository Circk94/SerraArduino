const int matrixGrade = 4;
int pinRighe[matrixGrade] = {12, 11, 10, 9};
int pinColonne[matrixGrade] = {8, 7, 6, 5};

void setup() {
  pinMode(pinColonne[0], OUTPUT);
  pinMode(pinColonne[1], OUTPUT);
  pinMode(pinColonne[2], OUTPUT);
  pinMode(pinColonne[3], OUTPUT);

  pinMode(pinRighe[0], INPUT);
  pinMode(pinRighe[1], INPUT);
  pinMode(pinRighe[2], INPUT);
  pinMode(pinRighe[3], INPUT);

  Serial.begin(9600);
}

void loop() {
  delay(100);
  readButtonMatrix();

}

void readButtonMatrix(){
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
          break;
        case 1:
          Serial.print("4");
          break;
        case 2:
          Serial.print("7");
          break;
        case 3:
          Serial.print("*");
          break;
        default:
          break;
      }
    }else if(digitalRead(pinRighe[0]) == LOW && digitalRead(pinRighe[1]) == HIGH && digitalRead(pinRighe[2]) == LOW && digitalRead(pinRighe[3]) == LOW){
      switch(i){
        case 0:
          Serial.print("2");
          break;
        case 1:
          Serial.print("5");
          break;
        case 2:
          Serial.print("8");
          break;
        case 3:
          Serial.print("0");
          break;
        default:
          break;
      }
    }else if(digitalRead(pinRighe[0]) == LOW && digitalRead(pinRighe[1]) == LOW && digitalRead(pinRighe[2]) == HIGH && digitalRead(pinRighe[3]) == LOW){
      switch(i){
        case 0:
          Serial.print("3");
          break;
        case 1:
          Serial.print("6");
          break;
        case 2:
          Serial.print("9");
          break;
        case 3:
          Serial.print("#");
          break;
        default:
          break;
      }
    }else if(digitalRead(pinRighe[0]) == LOW && digitalRead(pinRighe[1]) == LOW && digitalRead(pinRighe[2]) == LOW && digitalRead(pinRighe[3]) == HIGH){
      switch(i){
        case 0:
          Serial.print("A");
          break;
        case 1:
          Serial.print("B");
          break;
        case 2:
          Serial.print("C");
          break;
        case 3:
          Serial.print("D");
          break;
        default:
          break;
      }
    }else{
    }

    digitalWrite(pinColonne[i], LOW);
  }
}
