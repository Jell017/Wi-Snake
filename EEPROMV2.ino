#include <EEPROM.h>
#include <string.h>

typedef struct tabla_score {
  char nombre[8];
  int puntaje;
  int EEslot;
};

int paso;
tabla_score  AllScores[10], NewScore, scoretrucho;
tabla_score tabla_temporal;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("empezo setup");
  EEPROM.begin(512);
  Serial.println("llamamos a eeprom");
  
  //Serial.println(EEPROM.read(0));
  paso = 0;



/*
  scoretrucho.puntaje = 700;
  scoretrucho.EEslot = 8;
  scoretrucho.nombre[0]= 'U'; scoretrucho.nombre[1]= 'r'; scoretrucho.nombre[2]= 'i';   scoretrucho.nombre[3]= 'e'; scoretrucho.nombre[4]= 'l';
  EEPROM.put(scoretrucho.EEslot * sizeof(tabla_score) , scoretrucho);
  EEPROM.commit();

  scoretrucho.puntaje = 300;
  scoretrucho.EEslot = 5;
  scoretrucho.nombre[0]= '4'; scoretrucho.nombre[1]= 'B'; scoretrucho.nombre[2]= 'r';  scoretrucho.nombre[3]= 'a'; scoretrucho.nombre[4]= 'z';scoretrucho.nombre[5]= 'o'; scoretrucho.nombre[6]= 's'; 
  EEPROM.put(scoretrucho.EEslot * sizeof(tabla_score) , scoretrucho);
  EEPROM.commit();
*/

}

void loop() {
  //Serial.println("empezo loop");
  switch (paso) {
    case 0: {
        paso = 1;
        Serial.println("Ingrese nombre de nuevo score: ");
        break;
      }

    case 1: {
        if (Serial.available()) {  //Lee el primer caracter (del buffer)y sigue de largo.
          NewScore.nombre[0] = (char)Serial.read();
          paso = 2;
        }
        break;
      }

    case 2: {
        paso = 3;
        Serial.println("Ingrese el puntaje del nuevo score: ");
        break;
      }

    case 3: {
        if (Serial.available()) {  //Lee el primer caracter (del buffer)y sigue de largo.
          NewScore.puntaje = (int) Serial.parseInt();
          
          llenado_AllScores();
          comparacion_nuevo_score();
          table_sorting();
          paso = 4;
        }

        break;
      }

    case 4: {
        Serial.println("\n All scores:");
        for (int i = 0 ; i < 10 ; i++) {

          Serial.print(AllScores[i].nombre);
          Serial.print(" ");
          Serial.print(AllScores[i].puntaje);
          Serial.print(" ");
          Serial.println(AllScores[i].EEslot);
        }
        delay(500);
        //for (int j = 0 ; j < 10 ; j++) {
          
        //}
        paso = 0;

        break;
      }

    default: break;
  }
}


void llenado_AllScores() {
  int filas = 10;
  Serial.println("All eeprom:");
  for (int x = 0 ; x < filas ; x++) {
    EEPROM.get( x * sizeof(tabla_score) , AllScores[x]);
    tabla_temporal=AllScores[x];
          Serial.print(tabla_temporal.nombre);
          Serial.print(" ");
          Serial.print(tabla_temporal.puntaje);
          Serial.print(" ");
          Serial.println(tabla_temporal.EEslot);
  }
}

void comparacion_nuevo_score() {
  int filas = 10;
  int minimo = 15400;
  int Slot = 200;

  for (int i = 0 ; i < filas ; i++) {
    if (AllScores[i].puntaje < minimo) {
      minimo = AllScores[i].puntaje;
      Slot = AllScores[i].EEslot;
    }
  }
  if (NewScore.puntaje > minimo) {
    NewScore.EEslot = Slot;
    AllScores[Slot] = NewScore;
    EEPROM.put(NewScore.EEslot * sizeof(tabla_score) , NewScore);
    //Slot++;
    EEPROM.commit();
  }
}

void table_sorting() {
  bool b = true;
  int filas = 10;
  tabla_score score_temp;


  while (b == true) { //----------------------------------Bucle que se ejecuta mientas ocurra un cambio de posicion en los datos
    b = false;

    for (int i = 0 ; i <  filas-1; i++) {
      if (AllScores[i + 1].puntaje > AllScores[i].puntaje) {
        b = true;//----------------------------Hay un cambio de posicion
        score_temp = AllScores[i];
        AllScores[i] = AllScores[i + 1]; //-----Intercambio de la posicion de los datos
        AllScores[i + 1] = score_temp;
      }
    }
  }
}
