//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//

//El programa se encarga de manejar puertos GPIO para consultar sensores
// a través de comunicación Bluetooth
//Proyecto Velos

#include "BluetoothSerial.h"
#include "driver/gpio.h"
#include  <stdio.h>
#include  <string.h>


#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define LED_BUILTIN 2


BluetoothSerial SerialBT;
char codigo = ' ';
char PASSBTPORTON[5] = "6364";
//int PASSBTPORTON = 6364;
char clave[5] = {0};
//int clave = 0;
//char flagReintentar = '1';
int digito;
char digitoChar[] = {0};
int i = 0;
int SIZEPASS = 4;
char myString[] = {};
char estado = 'C';
String strBT = "";


//----------Funciones------------

void maquinaDeEstado(void);

void validarClave(void);

void switchCaseParametros(char, String);

void setup() {
    
	Serial.begin(115200);
	SerialBT.begin("ESP32-BTController"); //Bluetooth device name
  	Serial.println("The device started, now you can pair it with bluetooth!");
    //validarClave();
      
    pinMode(LED_BUILTIN, OUTPUT);

  
}

void loop() {
  
    maquinaDeEstado();

}

void maquinaDeEstado(void){


    if (SerialBT.available()) {

		strBT = SerialBT.readStringUntil('\n');//lee del puerto serie

		char param = strBT.charAt(0);//obtiene el parámetro
		String valor = strBT.substring(1);//obtiene el valor

		Serial.print("comando = ");
		Serial.println(param);
		Serial.print("valor = ");
		Serial.println(valor);

		switchCaseParametros(param, valor);//resuelve en base al parámetro y valor pasados

		strBT = "";//se deja limpio para la próxima lectura

	}

		/*

		if(strBT == "F100"){

			digitalWrite(LED_BUILTIN, HIGH);
			Serial.println("F100");

		}else if(strBT == "R100"){

			digitalWrite(LED_BUILTIN, LOW);
			Serial.println("R100");

		}else{
			
			Serial.println("Valor invalido");

		}
		*/



/*
		switch(strBT.compareTo("hola")){

			case 0:
				Serial.println("hola");

			break;
			case -1:
				Serial.println("no es hola");

			break;
			default:
				Serial.println("valor invalido");
			break;
		}
*/

    
  
}

void validarClave(void){

    int ret = 0;
    char leyenda[80] = {'\0'};
    int j = 0;
    char flagReintentar = '1';
    Serial.begin(115200);
    //SerialBT.begin("Porton Bluetooth"); //Bluetooth device name
    Serial.println("Ingrese la clave del sistema");
    while (SerialBT.available() == 0) {

        strcpy(leyenda, "Ingrese la clave");

        for(j = 0; j < sizeof(leyenda); j++){
            SerialBT.write(leyenda[j]);
            Serial.println(leyenda[j]);
        }
        delay(3000);
    }
    
    while(flagReintentar == '1'){
      
        i = 0;
        while(SerialBT.available() > 0){
            digito = SerialBT.read();//lee la clave enviada por la app (integer)
            sprintf(digitoChar, "%c", digito);
            clave[i] = digitoChar[0];
            i++;  
        }
        clave[i] = '\0';
        
        Serial.println(clave);
  
        ret = strcmp(clave, PASSBTPORTON);
       
        if(ret != 0){

            strcpy(leyenda, "Clave incorrecta, vuelva a intentarlo");

            for(j = 0; j < sizeof(leyenda); j++){
                SerialBT.write(leyenda[j]);
                Serial.println(leyenda[j]);
            }

            delay(3000);
            
        }else{

            strcpy(leyenda, "Bienvenido. Presione un botón para abrir o cerrar el portón");

            for(j = 0; j < sizeof(leyenda); j++){
                SerialBT.write(leyenda[j]);
                Serial.println(leyenda[j]);
            }

            flagReintentar = '0';
        }
    }


  
}


void switchCaseParametros(char charParamID, String valorParam){

  int inChar = 0;
  int index = 0;
  int valorParamLength = 0;
  int endIndex = 0;
  int modoDebug = 0;
  int consultarLecturas = 0;
  int correccionActivada = 0;
  uint8_t numSensor = 0;
  uint16_t direccion = 0;
  int scanActivado = 0;
  byte oldAddress = 0;
  byte newAddress = 0;
  int analizarLecturasCantidad = 0;
  int intercambioSensores = 0;
  int color = 0;
  String nombreSensor = "";
  int velocidad = 0;

  
  //valorParam = 
  valorParam.replace(0x0A,'\0');//Se filtra el caracter LF
  valorParam.replace(0x0D,'\0');//Se filtra el caracter CR

  switch(charParamID){
    case 'F':
      velocidad = valorParam.toInt();
      Serial.print("F velocidad: ");
      Serial.println(velocidad);
    break;
    case 'R':
      velocidad = valorParam.toInt();
      Serial.print("R velocidad: ");
      Serial.println(velocidad);
    break;


    default:
      Serial.println("Parámetro incorrecto");
    break;

  }  
}