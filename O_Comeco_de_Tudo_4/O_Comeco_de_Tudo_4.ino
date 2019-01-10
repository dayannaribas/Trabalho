//Bibliotecas
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h> //essa lib ta complicando com a String, ela precisa mandar o payload como byte[]
#include "DHT.h"

//Definições pinos
#define PIN_LED 15 // Pino de dados LED. no pino D8
#define PIN_VAL 13 // Pino de dados Valvula gas. no pino D7
#define PIN_DHT 12 // pino de dados DHT. No pino ESP8266 12 é D6
#define PIN_MQ2 A0 // pino analogico de dados
#define PIN_MQ9 16 // Pino de dados MQ9 digital. no pino D0

//Outras definições
#define DHT_TYPE DHT11 // DHT 11 é o tipo do sensor
#define READ_INTERVAL 3000 //intervalo de leitura (em milisegundos)


//Definições sensores
//#define SENSOR_MQ2 "mq2"     //Nome dos topicos que vem das solicitações feitas pelo MQTT
//#define SENSOR_MQ9 "mq9" 
//#define SENSOR_DHT11 "dht11" 
//#define LED1 "led1"
//#define IP "IP"
#define SENSOR_MQ2 "iot-2/cmd/mq2/fmt/json"
#define SENSOR_MQ9 "iot-2/cmd/mq9/fmt/json"
#define SENSOR_DHT11 "iot-2/cmd/dht11/fmt/json"
#define LED1 "iot-2/cmd/led1/fmt/json"
#define IP "iot-2/cmd/IP/fmt/json"  

//EXEMPLOS para estrutura de tópicos
//<comando>/<dispositivo>
//tele/dht11 //topico de envio publish
//stat/dth11 //topico de leitura subscribe
//tele/mq2   //topico de envio publish
//stat/m2    //topico de leitura subscribe
//power/led1 //comando liga/desliga

////conecta no servidor de testes do mosquitto
//const char* mqtt_server = "pedruino.com";

//dados obtidos ao cadastrar o novo dispopsitivo no Watson Iot
#define DEVICE_TOKEN "ySGDksiges&9ylFubp"
const String ORG = "riw7bn";
const String DEVICE_TYPE = "ESP8266-CASA";
const String DEVICE_ID = "ID001";
const String CLIENT_ID = "d:" + ORG + ":" + DEVICE_TYPE + ":" + DEVICE_ID;
const String MQTT_SERVER = ORG + ".messaging.internetofthings.ibmcloud.com";

DHT dht11Sensor(PIN_DHT, DHT_TYPE);

WiFiClient espclient;
long lastRead = 0;

void callback(char* topic, byte* payload, unsigned int length);
PubSubClient mqttClient(MQTT_SERVER.c_str(), 1883, callback, espclient);


void check_IP() {
  Serial.print("Solicitando IP: ");
  Serial.println(WiFi.localIP());
  String ipStr = WiFi.localIP().toString();
  
  String fullTopic = "day/tele/conection";
  char topic[fullTopic.length() + 1];
  char payload[ipStr.length() + 1];
  strcpy(topic, fullTopic.c_str());
  strcpy(payload, ipStr.c_str());
  
  mqttClient.publish(topic, payload);  //tópico MQTT de envio de informações para Broker 
}

void check_Mq2(){

   int nivel_sensor = 260;
   int sensorValue = analogRead(PIN_MQ2);
   String json;
   Serial.println(" MQ2 - " + String(sensorValue));
   
   if (sensorValue > nivel_sensor){
      json = "{\"ATENCAO PRESENCA DE GAS - VALVULA DE GAS DESLIGADA\":" + String(sensorValue)+ "}";
      digitalWrite(PIN_VAL, LOW);
      Serial.println(" VALVULA DE GAS DESLIGADA - " + String(digitalRead(PIN_VAL)));
   } else {
   
      digitalWrite(PIN_VAL, HIGH);
      json = "{\" VALVULA DE GAS LIGADA\":" + String(sensorValue)+ "}";
   }  
      String fullTopic = "day/tele/mq2";
      char topic[fullTopic.length() + 1];
      char payload[json.length() + 1];
      strcpy(topic, fullTopic.c_str());
      strcpy(payload, json.c_str());
      mqttClient.publish(topic, payload);  //tópico MQTT de envio de informações para Broker 
      Serial.println(" Valvula LIGADA - " + String(digitalRead(PIN_VAL))); 
   
}

void check_Mq9(){

   int nivel_sensor = 300;
   int sensorValue = digitalRead(PIN_MQ9);
   String json;
   Serial.println(" MQ9 - " + String(sensorValue));
//   
//   if (sensorValue > nivel_sensor){
//      json = "{\"ATENCAO PRESENCA DE FUMACA\":" + String(sensorValue)+ "}";
//      digitalWrite(PIN_VAL, LOW);
//   } else {
//   
//      digitalWrite(PIN_VAL, HIGH);
//      json = "{\"sem vazamento\":" + String(sensorValue)+ "}";
//   }   
//      String fullTopic = "day/tele/mq9";
//      char topic[fullTopic.length() + 1];
//      char payload[json.length() + 1];
//      strcpy(topic, fullTopic.c_str());
//      strcpy(payload, json.c_str());
//      mqttClient.publish(topic, payload);  //tópico MQTT de envio de informações para Broker  

}

void check_temperature() {

  int t = dht11Sensor.readTemperature();
    
  if (t == 2147483647) {
    t = dht11Sensor.readTemperature();
  }else{
      Serial.println("Temperatura solicitada");
      int t = dht11Sensor.readTemperature();   
      String temp = String(dht11Sensor.readTemperature());      
      String hum = String(dht11Sensor.readHumidity());
      String json = "{\"temperature\":" + temp + ",\"humidity\":" + hum + "}";
      String fullTopic = "day/tele/dht11";
      Serial.println(temp + "ºC / " + hum);
    
      char topic[fullTopic.length() + 1];
      char payload[json.length() + 1];
      strcpy(topic, fullTopic.c_str());
      strcpy(payload, json.c_str());
      mqttClient.publish(topic, payload);  //tópico MQTT de envio de informações para Broker  
  
  }
}

void check_sensors() {
  //TODO: Verifica todos os sensores
  
//  check_IP();
//  check_Mq2();
//  check_Mq9();
//  check_temperature();
  
}

void callback(char* topic, byte* payload, unsigned int length) {
//void callback(char* topic, char* payload, unsigned int length) {
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  String topicString = topic;
  
//  StaticJsonBuffer<30> jsonBuffer;
//  JsonObject& root = jsonBuffer.parseObject(payload);
// Serial.println(root);

 
  if (topicString.startsWith("day/stat")) { //Requisições externas de leitura (sensores)
    //TODO:Verifica qual o sensor requerido
    
    if (topicString.endsWith(SENSOR_DHT11)) {
      //TODO: "Força" leitura do sensor DHT11 e envia resultado
      check_temperature();

    } else if (topicString.endsWith(SENSOR_MQ2)) {
      check_Mq2();

    } else if (topicString.endsWith(SENSOR_MQ9)) {
      check_Mq9();
      
    } else if (topicString.endsWith(IP)){
      //TODO:Retorna o Ip para alteração de config. da wi-fi
      check_IP();
      
    } else {
      Serial.println("Sensor não encontrado: " + topicString);
    }

  } else if (topicString.startsWith("day/power")) {
    if (topicString.endsWith("led1")) {
      if (payload[0] == 49)
        digitalWrite(PIN_LED, HIGH); //ASCII VALUE OF '1' IS 49
      else if (payload[0] == 50)
        digitalWrite(PIN_LED, LOW);  //ASCII VALUE OF '2' IS 50
    }  
  } 
 
}

void setup_wifi() {
  WiFiManager wifiManager; //busca dados gravados na eeprom anteriormente e cria um access point
//  wifiManager.autoConnect("Domotica1"); //acessando o ip gerado no monitor serial, abrir no navegador,
//  Serial.println("Conectado!)");//escolher a rede para se conectar e trocar as credenciais

  Serial.println("CONECTANDO NA WiFi");
  WiFi.begin("MotoG3","123456789e");       //meu celular
  //WiFi.begin("Dayanna","94959697");       //minha casa

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(LED_BUILTIN, LOW); // Liga o LED
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH); // Desliga o LED
  }
}

////Função responsável pela conexão ao servidor MQTT
//void connectMQTTServer() {
//  Serial.println(CLIENT_ID+"use-token-auth"+DEVICE_TOKEN);
//  Serial.println("Conectando ao servidor MQTT...");
//  if (mqttClient.connect(CLIENT_ID.c_str(), "use-token-auth", DEVICE_TOKEN)) {
//     Serial.println("Conectado ao Broker MQTT...");
//     mqttClient.setCallback(callback);
//
////     mqttClient.subscribe(COMMAND_TOPIC_R1);
////     mqttClient.subscribe(COMMAND_TOPIC_R2);
////     digitalWrite(pinLED, HIGH);
//  } else {
//     Serial.print("erro = ");
//     Serial.println(mqttClient.state());
//     connectMQTTServer();
//  }
//}


void mqtt_reconect() {
  // Loop até estar reconectado
  while (!mqttClient.connected()) {
    Serial.print("Conectando-se ao servidor MQTT...");
   
//    // Cria um ID para o dispositivo. Ex: Pode ter um dispositivo na sala, no quarto, na cozinha, etc...
//    String clientId = "ESP8266Client-";
//    clientId += "cozinha1"; // >>>> EXEMPLO <<<<

    // Tenta conectar...
    if (mqttClient.connect(CLIENT_ID.c_str(), "use-token-auth", DEVICE_TOKEN)) {
      Serial.println("Conectado");

      // Uma vez conectado, publica uma mensagem de identificação...
      mqttClient.publish("iot-2/cmd/day/device/fmt/json", "");

      // ... e então "reassina" {resubscribe} todos os tópicos
      mqttClient.subscribe("iot-2/cmd/day/stat/#/fmt/json");
      mqttClient.subscribe("iot-2/cmd/day/power/#/fmt/json");

    } else {
      Serial.print("falhou, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Tentando novamente em 5 segundos");

      // Aguarda 5 segundos antes de tentar novamente
      delay(5000);
    }
  }
}

void setup() {
  
  pinMode(PIN_DHT, OUTPUT);//INPUT, usado na função pinMode, define que o pino será usado para entrada, ou seja, poderemos ler o sinal nesse pino
  pinMode(PIN_LED, OUTPUT);//OUTPUT é similar, apenas que é usado para saída e poderemos escrever no pino.
  pinMode(PIN_VAL, OUTPUT);
  pinMode(PIN_MQ9, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT); // Inicializa o LED_BUILTIN pin como output (saída)

  Serial.begin(115200);
  Serial1.begin(9600); //Debug

  //conexão com internet
  setup_wifi();
//  connectMQTTServer();
  
  //Inicia a leitura dos sensores
  dht11Sensor.begin();
}

void loop() {
  
  if (!mqttClient.connected()) {
    mqtt_reconect();
  }
  mqttClient.loop();
  

//  long now = millis();
//  if (now - lastRead < READ_INTERVAL) {
//    //TODO: lê os sensores
    check_sensors();
    delay(5000);
//  }
}
