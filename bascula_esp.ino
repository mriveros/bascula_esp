/* Autor: Marcos A. Riveros
 * mriveros.py@gmail.com
 * BASCULA ESP realiza una conexion inhalambrica via WIFI utilizando un ESP8266 donde
 * se monta un pequeño servidor web.
 * Este servidor sirve para exponer en el puerto 80 el valor de la variable incomedate.
 * La variable incomedate es el resultado del modulo serial INVELION INC. lector UHF.
 * 
*/
#include <SoftwareSerial.h>

SoftwareSerial espSerial(3, 2); // RX, TX
unsigned char ReadMulti[10] = {0XAA,0X00,0X27,0X00,0X03,0X22,0XFF,0XFF,0X4A,0XDD};
unsigned int timeSec = 0;
unsigned int timemin = 0;
unsigned int dataAdd = 0;
unsigned int incomedate = 0;
unsigned int parState = 0;
unsigned int codeState = 0;

// Declaraciones de funciones
void enviarRespuesta();
void leerRespuesta();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Hola mundo...");
  Serial.write(ReadMulti, 10);
  //SETUP ESP
  Serial.begin(115200);
  espSerial.begin(115200);
  Serial.println("Iniciando...");
  
  // Reinicia el ESP8266
  espSerial.println("AT+RST");
  delay(5000);  
  leerRespuesta();

  // Configura el ESP8266 en modo AP
  espSerial.println("AT+CWMODE=3"); // Modo AP
  delay(1000);
  leerRespuesta();

  // Configura el punto de acceso
  espSerial.println("AT+CWSAP=\"Bascula\",\"\",1,0"); 
  delay(2000);
  leerRespuesta();

  // Habilita múltiples conexiones y el servidor
  espSerial.println("AT+CIPMUX=1"); 
  delay(1000);
  leerRespuesta();
  
  //Habilitamos el puerto 80
  espSerial.println("AT+CIPSERVER=1,80"); 
  delay(1000);
  leerRespuesta();
  
  //obtener la ip del servidor esp (prueba)
  espSerial.println("AT+CIFSR"); 
  delay(1000);
  leerRespuesta();
}

void loop() {
  timeSec++;
  if (timeSec >= 50000) {
    timemin++;
    timeSec = 0;
    if (timemin >= 20) {
      timemin = 0;
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.write(ReadMulti, 10);
      digitalWrite(LED_BUILTIN, LOW); 
    }
  }

  if (Serial.available() > 0) {
    incomedate = Serial.read();
    if ((incomedate == 0x02) & (parState == 0)) {
      parState = 1;
    } else if ((parState == 1) & (incomedate == 0x22) & (codeState == 0)) {
      codeState = 1;
      dataAdd = 3;
    } else if (codeState == 1) {
      dataAdd++;
      if (dataAdd == 6) {
        Serial.print("RSSI:"); 
        Serial.println(incomedate, HEX); 
      } else if ((dataAdd == 7) | (dataAdd == 8)) {
        if (dataAdd == 7) {
          Serial.print("PC:"); 
          Serial.print(incomedate, HEX);
        } else {
          Serial.println(incomedate, HEX);
        }
      } else if ((dataAdd >= 9) & (dataAdd <= 20)) {
        if (dataAdd == 9) {
          Serial.print("EPC:"); 
        }
        Serial.print(incomedate, HEX);
      } else if (dataAdd >= 21) {
        Serial.println(""); 
        dataAdd = 0;
        parState = 0;
        codeState = 0;
      }
    } else {
      dataAdd = 0;
      parState = 0;
      codeState = 0;
    }
  }

  if (espSerial.available()) {
    String response = espSerial.readString();
    Serial.println("Respuesta del ESP8266: " + response);
    if (response.indexOf("GET") != -1) {
      enviarRespuesta();
    }
  }
}

void enviarRespuesta() {
  espSerial.println("AT+CIPSEND");
  delay(1000);
  leerRespuesta();

  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: text/html\r\n";
  response += "Connection: close\r\n\r\n";
  response += "<!DOCTYPE HTML>\r\n<html>\r\n";
  
  // Convertir el valor de incomedate a string
  response += "<p>incomedate: ";
  response += String(incomedate);  // enviamos el valor incomedate
  response += "</p>\r\n";
  response += "</html>\r\n";

  // Enviar la longitud de la respuesta
  espSerial.print("AT+CIPSEND=0," + String(response.length()) + "\r\n");
  delay(100);
  
  // Enviar la respuesta completa
  espSerial.print(response);
  delay(1000);
  leerRespuesta();
}

void leerRespuesta() {
  while (espSerial.available()) {
    Serial.write(espSerial.read());
  }
  Serial.println();
}
