//El programa se encarga de manejar puertos GPIO para consultar sensores
// a través de comunicación Bluetooth
//Proyecto Velos

#include "BluetoothSerial.h"
#include "driver/gpio.h"
#include  <stdio.h>
#include  <string.h>
#include <WiFi.h>

// LIBRERIAS PARA GRAFICAR
#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
#include <Adafruit_ILI9341.h>
#include <dummy.h>
#include "SPI.h"

// LIBRERIA PARA LEER GPS
#include <SoftwareSerial.h>
#include <TinyGPS.h>

// libreria para ADS1115
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

//***************************
//DEFINES
//***************************

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define LED_BUILTIN 2
#define pinMotorForward 16
#define pinMotorBack 17
#define pinMotorLeft 18
#define pinMotorRight 19
// Conexiones con la pantalla
#define TFT_DC 2
#define TFT_CS 16
//--------------Colores para evitar contraste------------------------
#define cfondo ILI9341_WHITE
#define clineas ILI9341_BLUE
#define ctexto ILI9341_BLACK
#define cdatos ILI9341_NAVY
#define celice ILI9341_NAVY
#define felice ILI9341_LIGHTGREY
#define cbverde ILI9341_DARKGREEN
#define cbamarillo ILI9341_OLIVE
#define cbnaranja ILI9341_ORANGE
#define cbrojo ILI9341_RED
#define cvelos ILI9341_WHITE

//#define lat pos[0];
//#define lon pos[1];

//***************************
//FUNCIONES
//***************************


// declaramamos las funciones generales:
void pantallainicial();
void pantallageneral();
void cargarvelocidad(int);
float leergps();
void leerADS();
void primerlecturaADS();
void leerADS();
void analisisbateria();
void cargarbateria();
void cargartiempo();
void maquinaDeEstado(void);
int validarClave(void);
void switchCaseParametros(char, String);
void accionarMotor(char, int);

//***************************
//OBJETOS
//***************************

BluetoothSerial SerialBT;
// definimos el tft como nombre de la CLase ILI9341.
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// defino dos puertos para transmitir por el GPS
TinyGPS gps;
SoftwareSerial softSerial(0, 15);
// definios los valores para el adc.
Adafruit_ADS1X15 ads;
// const float multiplicador = 0.0001869;


//***************************
//VARIABLES
//***************************

String strBT = "";
String clave = "1234";
int retornoClave = 0;

int cuentaForwardBack = 0;
int cuentaLeftRight = 0;

int lati;
int loni;
int veli;
int bati;

String mac = {};

int velocidad = 0, control, auxbat = 0, i = 0, timeminutos, contadordecontrol = 0;
int16_t adc0, adc1;
float auxiliar, velaux = 0, Vo = 0, Ao = 0, corriente, timehoras, corrientetotal, tension, primermedida;
float volt1, volt0, y, x, porcentaje, aver;
char linea[150], vel[10];
float matrizcorriente[5][7] = {{4.03, 0.94, 4.03, 3.09, 0.94, 4.03, 2.15}, {4.005, 0.945, 4.005, 3.06, 0.93, 3.86, 2.13}, {3.98, 0.97, 3.98, 3.01, 0.98, 3.99, 2.03}, {3.91, 1, 3.91, 2.91, 1.02, 3.93, 1.89}, {3.83, 1.02, 3.83, 2.81, 1.03, 3.84, 1.78}};
// PARA LA LECTURA DEL GPS
double pos[2]={0};
double lat = pos[0];
double lon = pos[1];
int flat = 25;
int flon = 26;
int temperature;




void setup() {
    
	Serial.begin(9600);
	softSerial.begin(9600);
	Wire.begin();

	mac = WiFi.macAddress();
	Serial.println(mac);

	//String nombre = "ESP32-BTControllerNEW " + mac;
	String nombre = "ESP32-MNC";
	Serial.println(nombre);
	//SerialBT.begin("ESP32-BTController"); //Bluetooth device name
	SerialBT.begin(nombre); //Bluetooth device name
  	Serial.println("The device started, now you can pair it with bluetooth!");
    //validarClave();
      
    pinMode(LED_BUILTIN, OUTPUT);
	pinMode(pinMotorBack, OUTPUT);
	pinMode(pinMotorForward, OUTPUT);
	pinMode(pinMotorLeft, OUTPUT);
	pinMode(pinMotorRight, OUTPUT);
	delay(10000);
	
	//tft.begin();
	//tft.setRotation(2);
	//pantallainicial();
	//pantallageneral();
	//ads.setGain(GAIN_ONE); // +/- 4.096V  1 bit = 0.125mV
	//ads.begin();
  	  
	primerlecturaADS();


  
}

void loop() {

/*
	while(retornoClave == 0){
		retornoClave = validarClave();
		
		if(retornoClave == 1){
			SerialBT.println("Comandos aceptados: ");
			SerialBT.println("F, B, L, R");
			
		}
		delay(10000);
	}
*/	
	lati = random (0,20);
	loni = random (0,20);
	veli = random (0,8);
	bati = random (0,99);
	
  
    maquinaDeEstado();

}

void maquinaDeEstado(void){


	SerialBT.print (lati);
	SerialBT.print ("|");
	SerialBT.print (loni);
	SerialBT.print ("|");
	SerialBT.print (veli);
	SerialBT.print ("|");
	SerialBT.print (bati);
	delay(1000);


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
  

  
  //valorParam = 
  valorParam.replace(0x0A,'\0');//Se filtra el caracter LF
  valorParam.replace(0x0D,'\0');//Se filtra el caracter CR

  switch(charParamID){
    //case 'F':
	case '8':
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
    //case 'B':
	case '2':
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

    //case 'L':
	case '4':
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
    //case 'R':
	case '6':
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

	case '5':
		/*
		if(cuentaLeftRight < 5){//aumenta o disminuye solo si esta dentro del rango permitido
			cuentaLeftRight++;
		}
		*/
		Serial.println("Detener");
	

		accionarMotor(charParamID, cuentaLeftRight);
    break;

	case '7':
		/*
		if(cuentaLeftRight < 5){//aumenta o disminuye solo si esta dentro del rango permitido
			cuentaLeftRight++;
		}
		*/
		Serial.println("Tomo Control");
	

		accionarMotor(charParamID, cuentaLeftRight);
    break;

	case '9':
		/*
		if(cuentaLeftRight < 5){//aumenta o disminuye solo si esta dentro del rango permitido
			cuentaLeftRight++;
		}
		*/
		Serial.println("Doy Control");
	

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

void pantallainicial()
{
	tft.fillScreen(ILI9341_WHITE);
	tft.setTextColor(ILI9341_BLUE);
	tft.setTextSize(3);
	tft.setCursor(10, 98);
	tft.print("INICIANDO...");
	delay(3000);
}
void pantallageneral()
{
	tft.fillScreen(cfondo);
	//------------Velocidad----------------
	tft.setTextColor(ctexto);
	tft.setTextSize(3);
	tft.setCursor(39, 0);
	tft.print("Velocidad");
	tft.setTextColor(cdatos);
	tft.setCursor(168, 30);
	tft.print("km/");
	tft.setCursor(168, 56);
	tft.print("hs");
	//------------Bateria---------------
	tft.setTextColor(ctexto);
	tft.setTextSize(3);
	tft.setCursor(57, 98);
	tft.print("Bateria");
	//------------Autonomia------------
	tft.setTextSize(3);
	tft.setCursor(39, 196);
	tft.print("Autonomia");
	//-------------Lineas-----------------
	tft.drawLine(0, 90, 240, 90, clineas);
	tft.drawLine(0, 188, 240, 188, clineas);
	tft.drawLine(0, 286, 240, 286, clineas);
	//------DIBUJO-----ELICE-------------------------
	// aleta arriba
	tft.fillCircle(38, 37, 8, celice);
	tft.fillCircle(38, 37, 6, felice);
	tft.fillTriangle(30, 38, 47, 38, 38, 58, celice);
	tft.fillTriangle(32, 38, 44, 38, 38, 56, felice);
	// aleta abajo izquierda
	tft.fillCircle(21, 69, 8, celice);
	tft.fillCircle(21, 69, 6, felice);
	tft.fillTriangle(21, 60, 27, 76, 38, 58, celice);
	tft.fillTriangle(21, 62, 25, 74, 38, 56, felice);
	// aleta abajo derecha
	tft.fillCircle(55, 69, 8, celice);
	tft.fillCircle(55, 69, 6, felice);
	tft.fillTriangle(55, 60, 49, 76, 38, 58, celice);
	tft.fillTriangle(55, 62, 51, 74, 38, 56, felice);
	// centro
	tft.fillCircle(38, 58, 7, celice);
	tft.fillCircle(38, 58, 5, felice);
	tft.fillCircle(38, 58, 3, celice);
	tft.fillCircle(38, 58, 1, felice);

	//------DIBUJO-----BATERIA----------------------
	tft.fillRoundRect(25, 127, 27, 54, 5, felice);
	tft.fillRoundRect(27, 129, 23, 50, 5, cfondo);
	tft.fillRoundRect(32, 125, 12, 4, 1, felice);
	//------DIBUJO-----RELOJ-----------------------

	tft.fillTriangle(28, 230, 48, 230, 38, 259, ctexto);
	tft.fillCircle(38, 255, 20, ctexto);
	tft.fillCircle(38, 255, 17, cfondo);
	tft.fillCircle(38, 255, 3, celice);
	tft.fillCircle(38, 255, 1, cfondo);
	tft.fillRoundRect(36, 240, 5, 19, 2, celice);
	tft.setTextColor(cdatos, cfondo);
	tft.setTextSize(6);
	tft.setCursor(137, 233);
	tft.print(":");
	tft.setCursor(78, 233);
	tft.print("00");
	tft.setCursor(160, 233);
	tft.print("00");
	//----------VELOS----------------------
	tft.fillRect(0, 286, 240, 34, celice);
	tft.setTextColor(cvelos);
	tft.setTextSize(3);
	tft.setCursor(25, 291);
	tft.print("V");
	tft.setCursor(68, 291);
	tft.print("E");
	tft.setCursor(111, 291);
	tft.print("L");
	tft.setCursor(154, 291);
	tft.print("O");
	tft.setCursor(197, 291);
	tft.print("S");
}

void cargarvelocidad(int vel)
{
	tft.setTextColor(cdatos, cfondo);
	tft.setTextSize(7);
	if (vel == -1)
	{
		tft.setCursor(78, 30);
		tft.print("--");
	}
	else if (vel < 1)
	{
		tft.setCursor(78, 30);
		tft.print("0");
		tft.setCursor(120, 30);
		tft.print("0");
	}
	else if (vel >= 1 && vel < 10)
	{
		tft.setCursor(78, 30);
		tft.print("0");
		tft.setCursor(120, 30);
		tft.print(vel);
	}
	else
	{
		tft.setCursor(78, 30);
		tft.print(vel);
	}
}

float leergps()
{

	bool newData = false;
	unsigned long chars;
	unsigned short sentences, failed;

	// Intentar recibir secuencia durante un segundo
	for (unsigned long start = millis(); millis() - start < 1000;)
	{
		while (softSerial.available())
		{
			char c = softSerial.read();
			if (gps.encode(c)) // Nueva secuencia recibida
				newData = true;
		}
	}
	float flat, flon;
	unsigned long age=0;
	if (newData)
	{
		aver = gps.f_speed_kmph();
		// void f_get_position(float *latitude, float *longitude, unsigned long *fix_age = 0);
		gps.f_get_position(&flat, &flon, &age ); // Guarda latitud y longitud
	/*
    // Asigno el valores de aver, flat y flon a características ble
    velocitydCharacteristic->setValue((uint8_t*)&aver, sizeof(aver));
    latitudeCharacteristic->setValue((uint8_t*)&flat, sizeof(flat));
    longitudeCharacteristic->setValue((uint8_t*)&flon, sizeof(flon));

    // notifico valores
    velocitydCharacteristic->notify();
    latitudeCharacteristic->notify();
    longitudeCharacteristic->notify();
		
	*/
    return aver;
	}
	else
	{
		return -1;
	}
}

void primerlecturaADS()
{
	//adc1 = ads.readADC_SingleEnded(1);
	//primermedida = ads.computeVolts(adc1); // primer medida para tener referencia
	Serial.print("la primer medida y referencia es: ");
	Serial.println(primermedida);
}

void leerADS()
{
	float auxvolt;
	adc1 = ads.readADC_SingleEnded(1); // mide corriente
	adc0 = ads.readADC_SingleEnded(0); // mide tensión
	volt0 = ads.computeVolts(adc0);	   // paso a volts
	Serial.print("Este es el valor de la tensión del divisor resistivo: ");
	Serial.println(volt0);
	volt1 = ads.computeVolts(adc1); // paso a volts
	Serial.print("Esta es la tensión del ADS que nos entrega desde la bateria: ");
	Serial.println(volt1);
	//------------CALCULO CORRIENTE---------------
	auxvolt = volt1 - primermedida;	 ///
	corrientetotal = auxvolt / 0.04; /// si son 3,3 se pone 0.027
	// YYY si se usa el otro dispositivo entonces es 0.037
	Serial.print("La corriente total es: ");
	Serial.println(corrientetotal);
	corriente = corrientetotal / 17;
	Serial.print("la corriente por cada columna de pilas: ");
	Serial.println(corriente);
	// porcentaje = (y*99)/0.8;
	//------------Calculo tensión-----------------
	// vamos a probar de otra forma
	// auxvolt = (volt0 * 122000)/22000;
	// tension = auxvolt /4; // con esto tengo la tensión de la pila
	tension = volt0;
	Serial.print("la tensión es: ");
	Serial.println(tension);
	// tension = tension -3.3;
	// porcentaje = (tension*99)/0.8;
	// Serial.println (porcentaje);
}

void analisisbateria()
{
	// mis parametros acá son tension, corriente, matrizcorriente[5][7], porcentaje,timeminutos, timehoras
	int fila, columna, valortensionmed, mascarga;
	float amperhora, auxtension;
	if (corriente <= 0.375)
		fila = 0;
	else if (corriente > 0.375 && corriente <= 0.75)
		fila = 1;
	else if (corriente > 0.75 && corriente <= 1.25)
		fila = 2;
	else if (corriente > 1.25 && corriente <= 2.2)
		fila = 3;
	else
		fila = 4;
	Serial.print("la fila elegida es: ");
	Serial.println(fila);
	if (tension > matrizcorriente[fila][3])
		mascarga = 1;
	else
		mascarga = 0;

	Serial.print("El valor del parametro mascarga es : ");
	Serial.println(mascarga);

	if (mascarga == 1)
	{
		// obtengo valor de amperhoras
		amperhora = 2 - (tension - matrizcorriente[fila][2]) / ((-1) * matrizcorriente[fila][1]);
		timehoras = amperhora / corriente; // obtengo el tiempo que me queda expresado en horas
		// timehoras = 2-timehoras; // le digo 2 horas menos ese tiempo que ya se consumio,
		// me queda el valor de tiempo que me queda de consumo.
		Serial.print("el tiempo de vida es: ");
		Serial.println(timehoras);
		if (timehoras <= 0.1)
			timehoras = 0;
		// calculo tensión
		if (tension > 2.8528)
			tension = 2.8528;
		if (tension <= 2.8528 && tension > 2.6972)
		{
			porcentaje = (6250.0 * tension) / 39 - 18575.0 / 52;
		}
		else if (tension <= 2.6972 && tension > 2.5329)
		{
			porcentaje = (6250.0 * tension) / 41 - 55125.0 / 164;
		}
		else if (tension <= 2.5329 && tension > 2.4206)
		{
			porcentaje = (3125.0 * tension) / 14 - 57725.0 / 112;
		}
		else if (tension <= 2.4206 && tension > 2.2477)
		{
			porcentaje = (25000.0 * tension) / 173 - 56200.0 / 173;
		}
		else
			porcentaje = -1;
		Serial.print("el valor del porcentaje es: ");
		Serial.println(porcentaje);
		// calculo de tensiṕn anterior
		//   tension = tension -3.3;
		// porcentaje = (tension*99)/0.8;
		// Serial.println (porcentaje);
	}
	else
	{
		// obtengo valor de amperhoras
		amperhora = 2 - (tension - matrizcorriente[fila][5]) / ((-1) * matrizcorriente[fila][4]);
		timehoras = amperhora / corriente; // obtengo el tiempo en HORAS.
		// timehoras = 2-timehoras; // le digo 2 horas menos ese tiempo que ya se consumio,
		Serial.print("el tiempo de vida es: ");
		Serial.println(timehoras);
		// me queda el valor de tiempo que me queda de consumo.
		if (timehoras <= 0.1)
			timehoras = 0;

		if (tension > 2.8528)
			tension = 2.8528;
		if (tension <= 2.8528 && tension > 2.6972)
		{
			porcentaje = (6250.0 * tension) / 39 - 18575.0 / 52;
		}
		else if (tension <= 2.6972 && tension > 2.5329)
		{
			porcentaje = (6250.0 * tension) / 41 - 55125.0 / 164;
		}
		else if (tension <= 2.5329 && tension > 2.4206)
		{
			porcentaje = (3125.0 * tension) / 14 - 57725.0 / 112;
		}
		else if (tension <= 2.4206 && tension > 2.2477)
		{
			porcentaje = (25000.0 * tension) / 173 - 56200.0 / 173;
		}
		else
			porcentaje = -1;
	}
}

void cargarbateria()
{
	int auxbat = 0;
	auxbat = (int)porcentaje; // PABLO: ESTE ES EL PORCENTAJE DE LA BATERIA

/*
  batteryCharacteristic->setValue((uint8_t*)&auxbat, sizeof(auxbat)); // Asigno el valor de auxbat a la característica
  batteryCharacteristic->notify(); //notifico este valor
*/
	Serial.print("este es el nivel de Bateria: ");
	Serial.println(auxbat);
	if (auxbat > 99)
		auxbat = 99;
	if (auxbat >= 75)
	{
		tft.setTextColor(cbverde, cfondo);
		tft.setTextSize(7);
		tft.setCursor(78, 128);
		tft.print(auxbat);
		tft.fillRoundRect(28, 131, 21, 11, 3, cbverde);
		tft.fillRoundRect(28, 143, 21, 11, 3, cbverde);
		tft.fillRoundRect(28, 155, 21, 11, 3, cbverde);
		tft.fillRoundRect(28, 167, 21, 11, 3, cbverde);
		// porcentaje
		tft.fillCircle(178, 134, 7, cbverde);
		tft.fillCircle(178, 134, 4, cfondo);
		tft.fillCircle(216, 172, 7, cbverde);
		tft.fillCircle(216, 172, 4, cfondo);
		tft.drawLine(172, 178, 222, 128, cbverde);
		tft.drawLine(172, 177, 221, 128, cbverde);
		tft.drawLine(172, 176, 220, 128, cbverde);
		tft.drawLine(173, 178, 222, 129, cbverde);
		tft.drawLine(174, 178, 222, 130, cbverde);
	}
	else if (auxbat < 75 && auxbat >= 50)
	{
		tft.setTextColor(cbamarillo, cfondo);
		tft.setTextSize(7);
		tft.setCursor(78, 128);
		tft.print(auxbat);
		tft.fillRoundRect(28, 131, 21, 11, 3, cfondo);
		tft.fillRoundRect(28, 143, 21, 11, 3, cbamarillo);
		tft.fillRoundRect(28, 155, 21, 11, 3, cbamarillo);
		tft.fillRoundRect(28, 167, 21, 11, 3, cbamarillo);
		// porcentaje
		tft.fillCircle(178, 134, 7, cbamarillo);
		tft.fillCircle(178, 134, 4, cfondo);
		tft.fillCircle(216, 172, 7, cbamarillo);
		tft.fillCircle(216, 172, 4, cfondo);
		tft.drawLine(172, 178, 222, 128, cbamarillo);
		tft.drawLine(172, 177, 221, 128, cbamarillo);
		tft.drawLine(172, 176, 220, 128, cbamarillo);
		tft.drawLine(173, 178, 222, 129, cbamarillo);
		tft.drawLine(174, 178, 222, 130, cbamarillo);
	}
	else if (auxbat < 50 && auxbat >= 25)
	{
		tft.setTextColor(cbnaranja, cfondo);
		tft.setTextSize(7);
		tft.setCursor(78, 128);
		tft.print(auxbat);
		tft.fillRoundRect(28, 131, 21, 11, 3, cfondo);
		tft.fillRoundRect(28, 143, 21, 11, 3, cfondo);
		tft.fillRoundRect(28, 155, 21, 11, 3, cbnaranja);
		tft.fillRoundRect(28, 167, 21, 11, 3, cbnaranja);
		// porcentaje
		tft.fillCircle(178, 134, 7, cbnaranja);
		tft.fillCircle(178, 134, 4, cfondo);
		tft.fillCircle(216, 172, 7, cbnaranja);
		tft.fillCircle(216, 172, 4, cfondo);
		tft.drawLine(172, 178, 222, 128, cbnaranja);
		tft.drawLine(172, 177, 221, 128, cbnaranja);
		tft.drawLine(172, 176, 220, 128, cbnaranja);
		tft.drawLine(173, 178, 222, 129, cbnaranja);
		tft.drawLine(174, 178, 222, 130, cbnaranja);
	}
	else if (auxbat <= 25 && auxbat >= 10)
	{
		tft.setTextColor(cbrojo, cfondo);
		tft.setTextSize(7);
		tft.setCursor(78, 128);
		tft.print(auxbat);
		tft.fillRoundRect(28, 131, 21, 11, 3, cfondo);
		tft.fillRoundRect(28, 143, 21, 11, 3, cfondo);
		tft.fillRoundRect(28, 155, 21, 11, 3, cfondo);
		tft.fillRoundRect(28, 167, 21, 11, 3, cbrojo);
		// porcentaje
		tft.fillCircle(178, 134, 7, cbrojo);
		tft.fillCircle(178, 134, 4, cfondo);
		tft.fillCircle(216, 172, 7, cbrojo);
		tft.fillCircle(216, 172, 4, cfondo);
		tft.drawLine(172, 178, 222, 128, cbrojo);
		tft.drawLine(172, 177, 221, 128, cbrojo);
		tft.drawLine(172, 176, 220, 128, cbrojo);
		tft.drawLine(173, 178, 222, 129, cbrojo);
		tft.drawLine(174, 178, 222, 130, cbrojo);
	}
	else if (auxbat < 10 && auxbat > 0)
	{
		tft.setTextColor(cbrojo, cfondo);
		tft.setTextSize(7);
		tft.setCursor(78, 128);
		tft.print("0");
		tft.print(auxbat);
		tft.fillRoundRect(28, 131, 21, 11, 3, cfondo);
		tft.fillRoundRect(28, 143, 21, 11, 3, cfondo);
		tft.fillRoundRect(28, 155, 21, 11, 3, cfondo);
		tft.fillRoundRect(28, 167, 21, 11, 3, cbrojo);
		// porcentaje
		tft.fillCircle(178, 134, 7, cbrojo);
		tft.fillCircle(178, 134, 4, cfondo);
		tft.fillCircle(216, 172, 7, cbrojo);
		tft.fillCircle(216, 172, 4, cfondo);
		tft.drawLine(172, 178, 222, 128, cbrojo);
		tft.drawLine(172, 177, 221, 128, cbrojo);
		tft.drawLine(172, 176, 220, 128, cbrojo);
		tft.drawLine(173, 178, 222, 129, cbrojo);
		tft.drawLine(174, 178, 222, 130, cbrojo);
	}
	/*else
		if (auxbat == -1)
			tft.setTextColor (cbrojo, cfondo);
			tft.setTextSize (7);
			tft.setCursor (78,128);
			tft.print ("--");
			tft.fillRoundRect(28, 131, 21, 11, 3, cfondo);
			tft.fillRoundRect(28, 143, 21, 11, 3, cfondo);
			tft.fillRoundRect(28, 155, 21, 11, 3, cfondo);
			tft.fillRoundRect(28, 167, 21, 11, 3, cfondo);
			//porcentaje
			tft.fillCircle (178, 134, 7, cbrojo);
			tft.fillCircle (178, 134, 4, cfondo);
			tft.fillCircle (216, 172, 7, cbrojo);
			tft.fillCircle (216, 172, 4, cfondo);
			tft.drawLine (172, 178, 222,128, cbrojo);
			tft.drawLine (172, 177, 221,128, cbrojo);
			tft.drawLine (172, 176, 220,128, cbrojo);
			tft.drawLine (173, 178, 222,129, cbrojo);
			tft.drawLine (174, 178, 222,130, cbrojo);*/
}

void cargartiempo()
{
	// int t;
	int hr, min;
	tft.setTextColor(cdatos, cfondo);
	tft.setTextSize(6);
	tft.setCursor(137, 233);
	tft.print(":");
	// t = ((Vo * 180)/3.3);
	hr = timehoras;
	if (hr > 3)
		hr = 3;
	min = (timehoras - hr) * 60;
	if (min > 60)
		min = 59;
	if (hr == 0)
	{
		tft.setCursor(78, 233);
		tft.print("00");
	}
	else if (hr < 10)
	{
		tft.setCursor(78, 233);
		tft.print("0");
		tft.setCursor(114, 233);
		tft.print(hr);
	}
	else
	{
		tft.setCursor(78, 233);
		tft.print(hr);
	}
	if (min == 0)
	{
		tft.setCursor(160, 233);
		tft.print("00");
	}
	else if (min < 10)
	{
		tft.setCursor(160, 233);
		tft.print("0");
		tft.setCursor(196, 233);
		tft.print(min);
	}
	else
	{
		tft.setCursor(160, 233);
		tft.print(min);
	}
}