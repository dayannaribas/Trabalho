//hUsI5u1fl/nPwUBZlwYnCc3669jbkuIAo3dRLEkJ

#include <WiFiManager.h>
WiFiClient espclient;

// Your access point's SSID and password
#define SSID = "DAIANA"
#define SSID_PASSWORD = "94959697"
 
// configure ESP as a station
WiFi.setmode(wifi.STATION)
WiFi.sta.config(SSID,SSID_PASSWORD)
WiFi.sta.autoconnect(1)
 
#define TWILIO_ACCOUNT_SID = "ACd3410200389cd3ff9cebe2c7c3b69251"
#define TWILIO_TOKEN = "bd3709ef6fa3b9558690e5d4af53ef7a"
 
#define HOST = "iot-https-relay.appspot.com"
#define URI = "/twilio/Calls.json"
 
void build_post_request(host, uri, data_table){
 
     local data = ""
     
     for param,value in pairs(data_table) do
          data = data .. param.."="..value.."&"
     end}
 
     request = "POST "..uri.." HTTP/1.1\r\n"..
     "Host: "..host.."\r\n"..
     "Connection: close\r\n"..
     "Content-Type: application/x-www-form-urlencoded\r\n"..
     "Content-Length: "..string.len(data).."\r\n"..
     "\r\n"..
     data
 
     print(request)
     
     return request
}//end
 
void function display(sck,response){
  
     print(response)
}//end
 
//When using send_sms: the "from" number HAS to be your twilio number.
// If you have a free twilio account the "to" number HAS to be your twilio verified number.

void function make_call(from,to,body){
 
     local data = {
      sid = TWILIO_ACCOUNT_SID,
      token = TWILIO_TOKEN,
      Body = string.gsub(body," ","+"),
      From = from,
      To = to
     }
     
     socket = net.createConnection(net.TCP,0)
     socket:on("receive",display)
     socket:connect(80,HOST)
 
     socket:on("connection",function(sck) 
       
          local post_request = build_post_request(HOST,URI,data)
          sck:send(post_request)
     end)     
}//end
 
void check_wifi(){
 local ip = wifi.sta.getip()
 
 if(ip==nil) then
   print("Connecting...")
 else
  tmr.stop(0)
  print("Connected to AP!")
  print(ip)
  make_call("18889866685","155534566809","Your house in on fire!") 
 end
 
}//end
 
tmr.alarm(0,2000,1,check_wifi)
