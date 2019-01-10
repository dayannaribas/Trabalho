//Bibliotecas
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <PubSubClient.h> //essa lib ta complicando com a String, ela precisa mandar o payload como byte[]
#include "DHT.h"

//Definições pinos
#define PIN_LED 4  // Pino de dados LED. no pino D7
#define PIN_VAL 13 // Pino de dados Valvula gas. no pino D5
#define PIN_DHT 12 // pino de dados DHT. No pino ESP8266 12 é D6
#define PIN_MQ2 A0 // pino analogico de dados
#define PIN_MQ9 5  // Pino de dados MQ9 digital. no pino D1 05

//Definições sensores
#define SENSOR_MQ2 "mq2"     //Nome dos topicos que vem das solicitações feitas pelo MQTT
#define SENSOR_MQ9 "mq9"
#define SENSOR_DHT11 "dht11"
#define LED1 "led1"
#define IP "IP"
#define STATUS_LED "ledLigado"

//Outras definições
#define DHT_TYPE DHT11 // DHT 11 é o tipo do sensor
#define READ_INTERVAL 3000 //intervalo de leitura (em milisegundos)


DHT dht11Sensor(PIN_DHT, DHT_TYPE);

//EXEMPLOS para estrutura de tópicos
//<comando>/<dispositivo>
//tele/dht11 //topico de envio publish
//stat/dth11 //topico de leitura subscribe
//tele/mq2   //topico de envio publish
//stat/mq2    //topico de leitura subscribe
//power/led1 //comando liga/desliga

//conecta no servidor 
const char* mqtt_server = "139.59.215.85";

WiFiClient espclient;
long lastRead = 0;

void callback(char* topic, byte* payload, unsigned int length);
PubSubClient mqttClient(mqtt_server, 1883, callback, espclient);


void check_Lamp() {
  
  int lampValue = digitalRead(PIN_LED);
  String json;
  json = "{\"lamp\":" + String(lampValue) + "}";
  String fullTopic = "day/tele/ledLigado";
  char topic[fullTopic.length() + 1];
  char payload[json.length() + 1];
  strcpy(topic, fullTopic.c_str());
  strcpy(payload, json.c_str());
  mqttClient.publish(topic, payload);  //tópico MQTT de envio de informações para Broker

}

void check_IP() {
  
  String ipStr = WiFi.localIP().toString();
  String fullTopic = "day/tele/conection";
  char topic[fullTopic.length() + 1];
  char payload[ipStr.length() + 1];
  strcpy(topic, fullTopic.c_str());
  strcpy(payload, ipStr.c_str());

  mqttClient.publish(topic, payload);  //tópico MQTT de envio de informações para Broker
}

void check_Mq2() {
  //rele é acionado em nivel LOW

  int sensorValue2 = analogRead(PIN_MQ2);
  String json;
  Serial.println(" MQ2 - " + String(sensorValue2));
  
  if (sensorValue2 > 100) {
    digitalWrite(PIN_VAL, HIGH);
  } else {
    digitalWrite(PIN_VAL, LOW);
  }
   
  json = "{\"VALVULA\":" + String(sensorValue2) + "}";
  String fullTopic = "day/tele/mq2";
  char topic[fullTopic.length() + 1];
  char payload[json.length() + 1];
  strcpy(topic, fullTopic.c_str());
  strcpy(payload, json.c_str());
  mqttClient.publish(topic, payload);  //tópico MQTT de envio de informações para Broker

}

void check_Mq9() {
  //rele é acionado em nivel LOW
  
  int sensorValue1 = analogRead(PIN_MQ9);
  String json;
  Serial.println(" MQ9 - " + String(sensorValue1));

  if (sensorValue1 > 0) {
    sensorValue1 = 30;
    digitalWrite(PIN_VAL, HIGH);
  } else {
    digitalWrite(PIN_VAL, LOW);
  }

  json = "{\"FUMACA\":" + String(sensorValue1) + "}";
  String fullTopic = "day/tele/mq9";
  char topic[fullTopic.length() + 1];
  char payload[json.length() + 1];
  strcpy(topic, fullTopic.c_str());
  strcpy(payload, json.c_str());
  mqttClient.publish(topic, payload);  //tópico MQTT de envio de informações para Broker

}

void check_temperature() {

  int t = dht11Sensor.readTemperature();

  if (t == 2147483647) {
    t = dht11Sensor.readTemperature();
  } 
  else {
    int t = dht11Sensor.readTemperature();
    String temp = String(dht11Sensor.readTemperature());
    String hum = String(dht11Sensor.readHumidity());
    String json = "{\"temperature\":" + temp + ",\"humidity\":" + hum + "}";
    String fullTopic = "day/tele/dht11";
    
    char topic[fullTopic.length() + 1];
    char payload[json.length() + 1];
    strcpy(topic, fullTopic.c_str());
    strcpy(payload, json.c_str());
    mqttClient.publish(topic, payload);  //tópico MQTT de envio de informações para Broker

  }
}

void check_sensors() {
  //TODO: Verifica todos os sensores

    check_IP();
    check_Mq2();
    check_Mq9();
    check_temperature();
    check_Lamp();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  String topicString = topic;

  if (topicString.startsWith("day/stat")) { 
    //TODO:Requisições externas de leitura (sensores)

    if (topicString.endsWith(SENSOR_DHT11)) {
      check_temperature();

    } else if (topicString.endsWith(SENSOR_MQ2)) {
      check_Mq2();

    } else if (topicString.endsWith(SENSOR_MQ9)) {
      check_Mq9();

    } else if (topicString.endsWith(IP)) {
      //TODO:Retorna o Ip para alteração de config. da wi-fi
      check_IP();

    } else if (topicString.endsWith(STATUS_LED)) {
      check_Lamp();

    } else {
      Serial.println("Sensor não encontrado: " + topicString);
    }
    
  } else if (topicString.startsWith("day/power")) {
    if (topicString.endsWith("led1")) {
      if (payload[0] == 49)
      {  digitalWrite(PIN_LED, HIGH); //ASCII VALUE OF '1' IS 49
      }else if (payload[0] == 50){
        digitalWrite(PIN_LED, LOW);  //ASCII VALUE OF '2' IS 50
      }
    }
  }
}

void setup_wifi() {
  WiFiManager wifiManager; //busca dados gravados na eeprom anteriormente e cria um access point
  //wifiManager.autoConnect("Domotica1"); //acessando o ip gerado no monitor serial, abrir no navegador,
  Serial.println("Conectado!)");//escolher a rede para se conectar e trocar as credenciais

  Serial.println("CONECTANDO NA WiFi");
  WiFi.begin("MotoG3","123456789e");       //meu celular
//  WiFi.begin("DAIANA", "94959697");      //minha casa

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void mqtt_reconect() {
  // Loop até estar reconectado
  while (!mqttClient.connected()) {
    Serial.print("Conectando-se ao servidor MQTT...");

    // Cria um ID para o dispositivo. Ex: Pode ter um dispositivo na sala, no quarto, na cozinha, etc...
    String clientId = "ESP8266Client-";
    clientId += "cozinha1"; //

    // Tenta conectar...
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("Conectado");

      // Uma vez conectado, publica uma mensagem de identificação...
      mqttClient.publish("day/device", "");

      // ... e então "reassina" {resubscribe} todos os tópicos
      mqttClient.subscribe("day/stat/#");
      mqttClient.subscribe("day/power/#");

    } else {
      Serial.print("falhou, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Tentando novamente em 5 segundos");

      // Aguarda 2 segundos antes de tentar novamente
      delay(2000);
    }
  }
}

void setup() {

  pinMode(PIN_DHT, OUTPUT);//INPUT, usado na função pinMode, define que o pino será usado para entrada, ou seja, poderemos ler o sinal nesse pino
  pinMode(PIN_LED, OUTPUT);//OUTPUT é similar, apenas que é usado para saída e poderemos escrever no pino.
  pinMode(PIN_VAL, OUTPUT);
  pinMode(PIN_MQ9, OUTPUT);
  pinMode(PIN_MQ2, OUTPUT);

  Serial.begin(115200);
  Serial1.begin(9600); //Debug

  //conexão com internet
  setup_wifi();

  //Inicia a leitura dos sensores
  dht11Sensor.begin();
}

void loop() {

  if (!mqttClient.connected()) {
    mqtt_reconect();
  }
  mqttClient.loop();

  //TODO: lê os sensores
  check_sensors();
  delay(2000);

}
