#include <ESP8266WiFi.h>
#include<PubSubClient.h>

//conecta no servidor de testes do mosquito
const char* mqtt_server="test.mosquitto.org";
WiFiClient espclient;

//declara a chamada
void callback(char* topic, byte* payload, unsigned int length);
PubSubClient client(mqtt_server,1883,callback,espclient);


void setup() {
  
  pinMode(4,OUTPUT);
  Serial.begin(115200);
  Serial.print(".");
  Serial.println("CONECTANDO NA WiFi");
  //WiFi.begin("MotoG3","123456789");         //SSID,PASSWORD 
   WiFi.begin("Dayanna","5499629652"); 
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  reconnect();

}


void callback(char* topic,byte* payload,unsigned int length1){    
Serial.print("RECEBENDO Do TOPICO ");
Serial.print(topic);
Serial.println(" O VALOR ASCII");

for(int i=0;i<length1;i++){
  Serial.print(payload[i]); 
  
}
if(payload[0]==49) 
  digitalWrite(4,HIGH);//ASCII VALUE OF '1' IS 49
else if(payload[0]==50)
  digitalWrite(4,LOW);//ASCII VALUE OF '2' IS 50
Serial.println();
}


void reconnect(){
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.println(".");
  }
  while(!client.connected()){
  if(client.connect("71896bcb.aed4c4")){// ID DO FOLOW
    Serial.println("CONECTADO");
    client.subscribe("ledcontrol");//NOME DO TOPICO
  }
    else{
      Serial.print("FALHA NO RECEBIMENTO ");
      Serial.println(client.state());
      delay(500);
    }
  } 
}
void loop() {
    if(!client.connected()){
      reconnect();
    }
    
    client.loop();

}
