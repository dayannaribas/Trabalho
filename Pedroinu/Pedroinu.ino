//
#include <ESP8266WiFi.h>

//conexão com internet
#include<PubSubClient.h>
#include <WiFiManager.h>
#include <DNSServer.h>
//conexão com internet
//
//LED
#define LEDPIN D8
#define TopicoLED "LED"
//LED

//DHT11
#include "DHT.h"
#define DHTPIN 12 // pino de dados DHT. No pino ESP8266 12 é D6
#define DHTTYPE DHT11 // DHT 11 é o tipo do sensor
DHT dht(DHTPIN, DHTTYPE);
float h, t, f;
String temp, hum, payload2;
char buf[256];
char EstadoSaida; //variável que armazena o estado atual da saída
#define TopicoDHT11 "DHT11"
#define StatusDHT11 "status/dht11"

#define SENSOR_MQ2   "mq2"
#define SENSOR_DHT11 "dht11"
//<comando>/<device>
void reconnect();

//tele/dht11 //topico de envio
//stat/dth11 //topico de leitura

//tele/mq2   //topico de envio
//stat/m2    //topico de leitura

//power/led1 //comando liga/desliga

//MQ2
#define TopicoMQ2 "MQ2"
int SensorMQ2 = A0; 
int nivel_sensor = 150;
//MQ2

//MQ9
//#define TopicoMQ9 "MQ9"
//int   SensorMQ9 = A0;//gas e monoxido de carbono
//MQ9

//defines - mapeamento de pinos do NodeMCU
//#define D0  16
//#define D1  5
//#define D2  4
//#define D3  0
//#define D4  2
//#define D5  14
//#define D6  12
//#define D7  13
//#define D8  15
//#define D9  3
//#define D10 1

//conecta no servidor de testes do mosquito
const char* mqtt_server = "pedruino.com";

WiFiClient espclient;
//
void callback(char* topic, byte* payload, unsigned int length);
PubSubClient client(mqtt_server, 1883, callback, espclient);

void setup() {

  pinMode(DHTPIN, OUTPUT); //sensor de temperatura
  pinMode(LEDPIN, OUTPUT); //led
  //pinMode(SensorMQ2, INPUT);// FUMAÇA MQ2
 
  Serial.begin(115200);
  Serial1.begin(9600);
  Serial.println();
  
  //conexão com internet
  WiFiManager wifiManager; //busca dados gravados na eeprom anteriormente e cria um access point
  wifiManager.autoConnect("Domotica1"); //acessando o ip gerado no monitor serial, abrir no navegador,
  Serial.println("Conectado!)");//escolher a rede para se conectar e trocar as credenciais
  //conexão com internet

  Serial.println("CONECTANDO NA WiFi");
//   WiFi.begin("MotoG3","123456789");       //meu celular
//   WiFi.begin("Dayanna","94959697");         //minha casa

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  reconnect();
  dht.begin();
}
void EnviaTemperatura(void)
{
  h = dht.readHumidity();
  t = dht.readTemperature();
  temp = String(t);
  hum = String(h);
  
//Serial.println(t);
//Serial.println(h);

  payload2 = "= TEMPERATURA ATUAL " + temp + "C E A UMIDADE " + hum + "% =";
  strcpy(buf, payload2.c_str());
  client.subscribe(StatusDHT11);// 
  
  if (t >= 40) {
    int state = client.publish(TopicoDHT11, buf);//tópico MQTT de envio de informações para Broker
    if (state) {
      Serial.println(payload2);
    }
  } else
  {
    Serial.println(payload2);
  }
}


void callback(char* topic, byte* payload, unsigned int length1) {
  Serial.print("RECEBENDO DO TOPICO { ");
  Serial.print(topic);
  Serial.print(" }");

//  Serial.print(payload);
  
  if (topic == TopicoLED) {
    if (payload[0] == 49)
      digitalWrite(LEDPIN, HIGH); //ASCII VALUE OF '1' IS 49
    else if (payload[0] == 50)
      digitalWrite(LEDPIN, LOW); //ASCII VALUE OF '2' IS 50
    Serial.println();
  }

  //DHT11
  if (topic = "day/stat/dht11") {
      EnviaTemperatura();
      Serial.println("temperatura solicitada");
      client.publish("day/tele/dht11", buf);
    }
  
}


void reconnect() {

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }

  while (!client.connected()) {
    //subscriptions
    client.subscribe("day/#");//Assina todos os topicos que começam com "day/" no servidor (broker) de MQTT
  }
}

void loop() {
  client.loop();
  delay(1000);

  if (!client.connected()) {
    reconnect();
  }
  EnviaTemperatura();
  EnviaMQ2();
}

void EnviaMQ2(void){
   int sensorValue = analogRead(SensorMQ2);
   Serial.println(" MQ-2 - " + String(sensorValue));
   //
   if (sensorValue > nivel_sensor){
      payload2 = " ATENCAO PRESENCA DE FUMACA ";
      strcpy(buf, payload2.c_str());
      int state = client.publish(TopicoMQ2, buf); //tópico MQTT de envio de informações para Broker
      if (state)
        Serial.println(payload2);
   }else 
      Serial.println("Tudo normal");

    
//  int valor_analogico = analogRead(SensorMQ2);
//  int valor_digital = digitalRead(SensorMQ2);

//  Serial.print("Pino D4 : ");
//  Serial.println(valor_digital);
//  Serial.print(" Pino A0 : ");
//  Serial.println(valor_analogico);
  //   Serial.println(" MQ2 - Fumaceira magnata = " + String(valor_digital));
//     if (valor_analogico > nivel_sensor)
//     {
//          payload2 = "= ATENCAO == PRESENÇA DE FUMAÇA =";
//          strcpy(buf, payload2.c_str());
//          int state = client.publish(TopicoMQ2, buf); //tópico MQTT de envio de informações para Broker
//          if (state)
//            Serial.println(payload2);
//      }
  //    else
  //       Serial.println(payload2);

}


void EnviaMQ9(void)
{
  
}
