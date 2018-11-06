//Upload to NodeMCU via Arduino IDE
//You can turn on and off built in leds of NodeMCU using Google Home and sinric.com

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> //  https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <ArduinoJson.h> // https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <StreamString.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;

#define MyApiKey "<API KEY>" // TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define MySSID "<WIFI NAME>" // TODO: Change to your Wifi network SSID
#define MyWifiPassword "<WIFI PASSWORD>" // TODO: Change to your Wifi network password

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes 


uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

void turnOn(String deviceId) {
  if (deviceId == "5axxxxxxxxxxxxxxxxxxx") // Device ID of first device
  {  
    digitalWrite(2, 0); //Makes GPIO2 LOW and turn on ESP8266 LED  
    Serial.print("Turn on device id: ");
    Serial.println(deviceId);

    
  }  
     else if (deviceId == "5axxxxxxxxxxxxxxxxxxx") // Device ID of second device
   { 
    
     digitalWrite(16, 0); //Makes GPIO16 LOW and turn on NodeMCU LED
     Serial.print("Turn on Device ID: ");
     Serial.println(deviceId);
  } 
}

void turnOff(String deviceId) {
   if (deviceId == "5axxxxxxxxxxxxxxxxxxx") // Device ID of first device
   {  

    digitalWrite(2, 1); //Makes GPIO16 HIGH and turn on NodeMCU LED
     Serial.print("Turn off Device ID: ");
     Serial.println(deviceId);

     
   }
     else if (deviceId == "5axxxxxxxxxxxxxxxxxxx") // Device ID of second device
  { 
    digitalWrite(16, 1); //Makes GPIO16 HIGH and turn on NodeMCU LED
    Serial.print("Turn off device id: ");
    Serial.println(deviceId);
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;    
      Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
      Serial.printf("Waiting for commands from sinric.com ...\n");        
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[WSc] get text: %s\n", payload);
        // Example payloads

        // For Switch  types
        // {"deviceId":"5be14ddf41f3eb3a9de21848","action":"action.devices.commands.OnOff","value":{"on":true}} // https://developers.google.com/actions/smarthome/traits/onoff
        // {"deviceId":"5be14ddf41f3eb3a9de21848","action":"action.devices.commands.OnOff","value":{"on":false}}

        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload); 
        String deviceId = json ["deviceId"];     
        String action = json ["action"];
        
        if(action == "action.devices.commands.OnOff") { // Switch 
            String value = json ["value"]["on"];
            Serial.println(value); 
            
            if(value == "true") {
                turnOn(deviceId);
            } else {
                turnOff(deviceId);
            }
        }
        else if (action == "test") {
            Serial.println("[WSc] received test command from sinric.com");
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
  }
}

void setup() {

  pinMode(2, OUTPUT); // Initialize GPIO2 pin as an output
  pinMode(16, OUTPUT); // Initialize GPIO16 pin as an output
  Serial.begin(115200);
  
  WiFiMulti.addAP(MySSID, MyWifiPassword);
  Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(MySSID);  

  // Waiting for Wifi connect
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if(WiFiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi connected. ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  // server address, port and URL
  webSocket.begin("iot.sinric.com", 80, "/"); //"iot.sinric.com", 80

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  
  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);   // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets
}

void loop() {
  webSocket.loop();
  
  if(isConnected) {
      uint64_t now = millis();
      
      // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
      if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
          heartbeatTimestamp = now;
          webSocket.sendTXT("H");          
      }
  }   
}
 
