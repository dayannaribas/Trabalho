#include "ESP8266WiFi.h"
#include <WiFiManager.h>
#include <PubSubClient.h> //essa lib ta complicando com a String, ela precisa mandar o payload como byte[]
//
#define IP "IPbtn"
#define PIN_BTN 14
//
// WiFi parameters
const  char * ssid = "DAIANA" ;
const  char * password = "94959697" ;


//conecta no servidor de testes do mosquitto
const char* mqtt_server = "139.59.215.85";


WiFiClient espclient;


void callback(char* topic, byte* payload, unsigned int length);
PubSubClient mqttClient(mqtt_server, 1883, callback, espclient);


void check_IP() {
  Serial.print("Solicitando IP: ");
  Serial.println(WiFi.localIP());
  String ipStr = WiFi.localIP().toString();

  String fullTopic = "day/tele/conectionBtn";
  char topic[fullTopic.length() + 1];
  char payload[ipStr.length() + 1];
  strcpy(topic, fullTopic.c_str());
  strcpy(payload, ipStr.c_str());

  mqttClient.publish(topic, payload);  //tópico MQTT de envio de informações para Broker
}

void check_button() {
  int button = digitalRead(PIN_BTN);
  String json = "{\"buttonpanic\":" + String(button) + "}";
  String fullTopic = "day/tele/buttonpanic";
  char topic[fullTopic.length() + 1];
  char payload[json.length() + 1];
  strcpy(topic, fullTopic.c_str());
  strcpy(payload, json.c_str());
  
  mqttClient.publish(topic, payload);  //tópico MQTT de envio de informações para Broker 
  Serial.println(json);
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
    clientId += "btn"; // >>>> EXEMPLO <<<<

    // Tenta conectar...
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("Conectado");

      // Uma vez conectado, publica uma mensagem de identificação...
      mqttClient.publish("day/device", "");

      // ... e então "reassina" {resubscribe} todos os tópicos
      mqttClient.subscribe("day/stat/buttonpanic");


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
    pinMode(LED_BUILTIN, OUTPUT); // Inicializa o LED_BUILTIN pin como output (saída)
    Serial.begin(9600);
    
    // Connect to WiFi
    WiFiManager wifiManager; //busca dados gravados na eeprom anteriormente e cria um access point
//    wifiManager.autoConnect("Domotica2"); //acessando o ip gerado no monitor serial, abrir no navegador,
    //Descomentar para usar os parametros de conexão
    
    WiFi.begin("MotoG3","123456789e");       //meu celular
//  WiFi.begin("DAIANA", "94959697");        //minha casa
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

    Serial.println(digitalRead(PIN_BTN)); 
    digitalWrite(LED_BUILTIN, LOW); // Liga o LED
    delay(100); 
    check_button();
    check_IP();
    digitalWrite(LED_BUILTIN, HIGH); // Desliga o LED  
    delay(1900);                                                                                                                                                                                                                                                                                                                                       
}
