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
#define pinMotorForward 16
#define pinMotorBack 17
#define pinMotorLeft 18
#define pinMotorRight 19

BluetoothSerial SerialBT;
//char codigo = ' ';
//char PASSBTPORTON[5] = "6364";
//int PASSBTPORTON = 6364;
//char clave[5] = {0};
//int clave = 0;
//char flagReintentar = '1';
//int digito;
//char digitoChar[] = {0};
//int i = 0;
//int SIZEPASS = 4;
//char myString[] = {};
//char estado = 'C';
String strBT = "";
String clave = "1234";
int retornoClave = 0;
/*
int cuentaForward = 0;
int cuentaBack = 0;
int cuentaLeft = 0;
int cuentaRight = 0;
*/

int cuentaForwardBack = 0;
int cuentaLeftRight = 0;


//----------Funciones------------

void maquinaDeEstado(void);

int validarClave(void);

void switchCaseParametros(char, String);

void accionarMotor(char, int);

void setup() {
    
	Serial.begin(115200);
	SerialBT.begin("ESP32-BTController"); //Bluetooth device name
  	Serial.println("The device started, now you can pair it with bluetooth!");
    //validarClave();
      
    pinMode(LED_BUILTIN, OUTPUT);
	pinMode(pinMotorBack, OUTPUT);
	pinMode(pinMotorForward, OUTPUT);
	pinMode(pinMotorLeft, OUTPUT);
	pinMode(pinMotorRight, OUTPUT);



  
}

void loop() {

	while(retornoClave == 0){
		retornoClave = validarClave();
		
		if(retornoClave == 1){
			SerialBT.println("Comandos aceptados: ");
			SerialBT.println("F, B, L, R");
			
		}
		delay(10000);
	}
	
	
  
    maquinaDeEstado();

}

void maquinaDeEstado(void){


    if (SerialBT.available()) {

		strBT = SerialBT.readStringUntil('\n');//lee del puerto serie



		char param = strBT.charAt(0);//obtiene el parámetro
		//String valor = strBT.substring(1);//obtiene el valor

		Serial.print("comando = ");
		Serial.println(param);

		//Serial.print("valor = ");
		//Serial.println(valor);

		switchCaseParametros(param, "1");//resuelve en base al parámetro y valor pasados

		strBT = "";//se deja limpio para la próxima lectura

	}
   
  
}

int validarClave(void){

    char flagReintentar = 1;
	String claveIngresada = {};
	int cantidadReintentos = 0;

    Serial.println("Ingrese la clave del sistema");
	SerialBT.println("Ingrese la clave del sistema");

	//while(flagReintentar == 1 && cantidadReintentos < 3){

		while (SerialBT.available() > 0) {

			claveIngresada = SerialBT.readStringUntil('\n');

			//claveIngresada.replace('2','9');//Se filtra el caracter LF
			claveIngresada.remove(4);//se agrega el NULL para cortar el string
			//claveIngresada.replace('\n','\0');//Se filtra el caracter CR
			//claveIngresada.setCharAt(4, '\0');//se agrega el NULL para cortar el string
			Serial.println("CLAVE INGRESADA = ");
			Serial.println(claveIngresada);
			SerialBT.println("CLAVE INGRESADA = ");
			SerialBT.println(claveIngresada);

			if(claveIngresada == clave){

				Serial.println("CLAVE CORRECTA");
				SerialBT.println("CLAVE CORRECTA");
				//flagReintentar = 0;
				return 1;

			}else{
				
				Serial.println("CLAVE INCORRECTA, INTENTE DE NUEVO");
				SerialBT.println("CLAVE INCORRECTA, INTENTE DE NUEVO");
				//flagReintentar = 1;
				//cantidadReintentos++;
				//Serial.println("Se superó la cantidad de intentos. Debe resetear el sistema");
				//SerialBT.println("Se superó la cantidad de intentos. Debe resetear el sistema");
				return 0;
			}


			claveIngresada = "";//se borra la clave ingresada para un futuro ingreso


		}

		return 0;



	//}


  
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
		if(cuentaForwardBack < 5){//aumenta o disminuye solo si esta dentro del rango permitido
			cuentaForwardBack++;
		}
		
		Serial.print("F: ");
		Serial.println(cuentaForwardBack);

		if(cuentaForwardBack == 5){

			Serial.println("F Maxima");
			SerialBT.println("F Maxima");

		}

		accionarMotor(charParamID, cuentaForwardBack);

      //velocidad = valorParam.toInt();
      //Serial.print("F velocidad: ");
      //Serial.println(velocidad);
    break;
    case 'B':
		if(cuentaForwardBack > -5){//aumenta o disminuye solo si esta dentro del rango permitido
			cuentaForwardBack--;
		}
		
		Serial.print("B: ");
		Serial.println(cuentaForwardBack);


	  	if(cuentaForwardBack == -5){
			
			Serial.println("B Maxima");
			SerialBT.println("B Maxima");

		}

		accionarMotor(charParamID, cuentaForwardBack);	
    break;

    case 'L':
		if(cuentaLeftRight > -5){//aumenta o disminuye solo si esta dentro del rango permitido
			cuentaLeftRight--;
		}
		
		Serial.print("L: ");
		Serial.println(cuentaLeftRight);

		if(cuentaLeftRight == -5){

			Serial.println("L Maxima");
			SerialBT.println("L Maxima");

		}

		accionarMotor(charParamID, cuentaLeftRight);

      //velocidad = valorParam.toInt();
      //Serial.print("F velocidad: ");
      //Serial.println(velocidad);
    break;
    case 'R':
		if(cuentaLeftRight < 5){//aumenta o disminuye solo si esta dentro del rango permitido
			cuentaLeftRight++;
		}
		
		Serial.print("R: ");
		Serial.println(cuentaLeftRight);


	  	if(cuentaLeftRight == 5){
			
			Serial.println("R Maxima");
			SerialBT.println("R Maxima");

		}	

		accionarMotor(charParamID, cuentaLeftRight);
    break;


    default:
      Serial.println("Parámetro incorrecto");
    break;

  }  
}


void accionarMotor(char param, int cuenta){

	switch (param)
	{
	case 'F':
		//apagar LEDs de señalizadores que no corresponden y encender el indicado
		digitalWrite(pinMotorLeft, LOW);
		digitalWrite(pinMotorRight, LOW);
		digitalWrite(pinMotorBack, LOW);
		digitalWrite(pinMotorForward, HIGH);

	break;
	case 'B':
		//apagar LEDs de señalizadores que no corresponden y encender el indicado
		digitalWrite(pinMotorLeft, LOW);
		digitalWrite(pinMotorRight, LOW);
		digitalWrite(pinMotorBack, HIGH);
		digitalWrite(pinMotorForward, LOW);

	break;
	case 'L':
		//apagar LEDs de señalizadores que no corresponden y encender el indicado
		digitalWrite(pinMotorLeft, HIGH);
		digitalWrite(pinMotorRight, LOW);
		digitalWrite(pinMotorBack, LOW);
		digitalWrite(pinMotorForward, LOW);

	break;
	case 'R':
		//apagar LEDs de señalizadores que no corresponden y encender el indicado
		digitalWrite(pinMotorLeft, LOW);
		digitalWrite(pinMotorRight, HIGH);
		digitalWrite(pinMotorBack, LOW);
		digitalWrite(pinMotorForward, LOW);

	break;
	
	default:
		break;
	}



}