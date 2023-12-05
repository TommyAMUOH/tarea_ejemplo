#include <StarterKitNB.h>
#include <SparkFun_SHTC3.h>

StarterKitNB SK;   
SHTC3 mySHTC3; 

//Definiciones
#define PIN_VBAT WB_A0
#define VBAT_MV_PER_LSB (0.73242188F) // 3.0V ADC range and 12 - bit ADC resolution = 3000mV / 4096
#define VBAT_DIVIDER_COMP (1.73)      // Compensation factor for the VBAT divider, depend on the board
#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)
#define LGREEN WB_LED1
#define LBLUE WB_LED2

//Variables Conectividad Entel
String tech = "NBIoT";            // "NBIoT" para NB-IoT | "CATM1" para CAT-M1
String APN = "m2m.entel.cl";      
String username_apn = "entelpcs";
String password_apn = "entelpcs";
String banda = "B28 LTE";         //Banda 2: B2 LTE | Banda 28: B28 LTE | Ambas bandas: B2 LTE, B28 LTE
//Variables Adafruit
String host = "io.adafruit.com";
String port = "1883";
String topic1 = "madelaine22/feeds/temp";
String topic2 = "madelaine22/feeds/hum";
String username = "madelaine22";
String password = "aio_gxks81CfOi599rfh1BODcBbg9vDn";
//Variables auxiliares JSON
String lat = "";
String lon = "";
String ele = "";
String resp = "";
String op = "";
String red = "";
String band = "";
String RSSI = "";
String RSRP = "";
String RSRQ = "";
String SINR = "";
String PCI = "";
String msg = "";
String value;
//Variables auxiliares generales
int indexVariables;
int indexVariablesNext;
int tries = 2;
int intentos = 0;
bool GPSretry = false;
// temp&hum
String hum = "";
String temp = "";
void errorDecoder(SHTC3_Status_TypeDef message)             
{
  switch(message)
  {
    case SHTC3_Status_Nominal : Serial.println("Nominal"); break;
    case SHTC3_Status_Error : Serial.println("Error"); break;
    case SHTC3_Status_CRC_Fail : Serial.println("CRC Fail"); break;
    default : Serial.println("Unknown return code"); break;
  }
}

void setup(){
  Serial.begin(115200);
  //Configuración de la conexión
  SK.Setup(true);   
  delay(2000);
  errorDecoder(mySHTC3.begin()); 
  delay(500);
  SK.Connect(APN, banda, tech);
  delay(500);
  SK.StopPSM();
  //esp_sleep_enable_timer_wakeup(120 * 1000000); //Deepsleep configurado en un minuto: 60 * 1000000[microsegundos];
}

void loop(){ 
  //Se comprueba la conexión a la red. Se reintenta en caso contrario.
  if (!SK.ConnectionStatus()){ 
    SK.Reconnect(APN, banda, tech);
    delay(2000);
  }

  SHTC3_Status_TypeDef result = mySHTC3.update();
  hum = String(mySHTC3.toPercent());
  temp = String(mySHTC3.toDegC());
  Serial.println("Temperatura: "+temp);
  Serial.println("Humedad: "+hum);


  // Se inicia el envío del mensaje abriendo MQTT
  resp = SK.bg77_at((char *)"AT+QMTOPEN=0,\"io.adafruit.com\",1883", 500, true); 
  delay(1000);
  if (resp.indexOf("QMTOPEN: 0,0")==-1){
    resp = SK.ReadSerial("QMTOPEN: 0,0", 30000);
    delay(1000);
  }
  Serial.println("MQTT abierto");

  // Se conecta a MQTT
  resp = SK.bg77_at((char *)"AT+QMTCONN=0,\"0\",\"madelaine22\",\"aio_gxks81CfOi599rfh1BODcBbg9vDn\"", 500, true);
  delay(1000);
  if (resp.indexOf("QMTCONN: 0,0,0")==-1){
    resp = SK.ReadSerial("QMTCONN: 0,0,0", 30000);
    delay(1000);
  }
  Serial.println("MQTT conectado");

  // Se publica data usando MQTT
  String envio = "AT+QMTPUBEX=0,0,0,0,"; //Verificar cambio en el que se toma el tópico según lo dado
  envio.concat("\"");
  envio.concat(topic1);
  envio.concat("\",\"");
  envio.concat(temp);
  envio.concat("\"");
  char envio_char[envio.length()+1];
  envio.toCharArray(envio_char, envio.length()+1);
  Serial.println(envio_char);
  resp = SK.bg77_at(envio_char, 500, true);
  delay(1000);
  if (resp.indexOf("QMTPUB: 0,0,0")==-1){
    resp = SK.ReadSerial("QMTPUB: 0,0,0", 30000);
    delay(1000);
  }
  delay(1000);
  envio = "AT+QMTPUBEX=0,0,0,0,"; //Verificar cambio en el que se toma el tópico según lo dado
  envio.concat("\"");
  envio.concat(topic2);
  envio.concat("\",\"");
  envio.concat(hum);
  envio.concat("\"");
  char envio_char1[envio.length()+1];
  envio.toCharArray(envio_char1, envio.length()+1);
  Serial.println(envio_char1);
  resp = SK.bg77_at(envio_char1, 500, true);
  delay(1000);
  if (resp.indexOf("QMTPUB: 0,0,0")==-1){
    resp = SK.ReadSerial("QMTPUB: 0,0,0", 30000);
    delay(1000);
  }
  Serial.println("Mensaje publicado");

  // Se desconecta al broker MQTT
  resp = SK.bg77_at((char *)"AT+QMTDISC=0", 500, true);
  delay(1000);
  if (resp.indexOf("QMTDISC: 0,0")==-1){
    resp = SK.ReadSerial("QMTDISC: 0,0", 30000);
    delay(1000);
  }
  Serial.println("MQTT cerrado");

  GPSretry = false;
}
