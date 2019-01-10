#include "ESP8266WiFi.h"
#include <WiFiManager.h>
#include <PubSubClient.h> //essa lib ta complicando com a String, ela precisa mandar o payload como byte[]
//
#define IP "IP"
#define PIN_BTN 14
//
// WiFi parameters
const  char * ssid = "DAIANA" ;
const  char * password = "94959697" ;


//conecta no servidor de testes do mosquitto
const char* mqtt_server = "pedruino.com";


WiFiClient espclient;


void callback(char* topic, byte* payload, unsigned int length);
PubSubClient mqttClient(mqtt_server, 1883, callback, espclient);


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

void check_button() {
  Serial.print("Solicitando Status do botão: ");
  String button = String(digitalRead(PIN_BTN));
  String fullTopic = "day/tele/buttonpanic";
  char topic[fullTopic.length() + 1];
  char payload[2];
  strcpy(topic, fullTopic.c_str());
  strcpy(payload, button.c_str());
  
  mqttClient.publish(topic, payload);  //tópico MQTT de envio de informações para Broker 
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

 
  if (topicString.startsWith("day/stat")) { //Requisições externas de leitura (sensores)
    //TODO:Verifica qual o sensor requerido
    
    if (topicString.endsWith("button")) {
      //TODO: "Força" leitura do botão e envia resultado
      check_button();
      
    } else if (topicString.endsWith(IP)){
      //TODO:Retorna o Ip para alteração de config. da wi-fi
      check_IP();
    } 
  }
}

void mqtt_reconect() {
  // Loop até estar reconectado
  while (!mqttClient.connected()) {
    Serial.print("Conectando-se ao servidor MQTT...");

    // Cria um ID para o dispositivo. Ex: Pode ter um dispositivo na sala, no quarto, na cozinha, etc...
    String clientId = "ESP8266Client-";
    clientId += "cozinha1"; // >>>> EXEMPLO <<<<

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

      // Aguarda 5 segundos antes de tentar novamente
      delay(5000);
    }
  }
}

void setup(void){ 
    // Start Serial
    pinMode (PIN_BTN,INPUT);  
    Serial.begin(9600);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi conectado");
    
    // Print the IP address
    Serial.println(WiFi.localIP());
}

void loop() {

   if (!mqttClient.connected()) {
    mqtt_reconect();
    }
    mqttClient.loop();

    check_button();
    
    Serial.println (digitalRead(PIN_BTN)); 
    delay(1000);                                                                                                                                                                                                                                                                                                                                         
}
