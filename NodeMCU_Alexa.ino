#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> //  https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <ArduinoJson.h> // https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <StreamString.h>
#include <math.h>       /* floor */
#include <HSBColor.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;


///////////WiFi Setup/////////////

#define MyApiKey "----" // TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define MySSID "----" // TODO: Change to your Wifi network SSID
#define MyWifiPassword "----" // TODO: Change to your Wifi network password

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes 
const int relayPin = D6;
int red_light_pin= D1;
int green_light_pin = D2;
int blue_light_pin = D3;
const uint16_t kIrLed = D5;
uint64_t heartbeatTimestamp = 0;
bool isConnected = false;
bool isDISCO_ON = false;

String AIRCORN = "5f60627ed1e9084a70846816";
String LAMP = "5f5c6874d1e9084a7083dfba";
String DISCO = "5f605473d1e9084a70846635";
String AC_HIGH = "5f6062a5d1e9084a7084681c";
String AC_LOW = "5f606419d1e9084a7084684e";
int rgb_arr[3] = {0, 0, 0};
int incomingByte = 0; // for incoming serial data


//USE 
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

    uint16_t OFF[179] = {
3184, 1600, 384, 412, 384, 1204, 384, 408, 384, 412, 384, 1204, 380, 412, 384, 1204, 384, 412, 384, 408, 384, 1204, 384, 1204, 384, 1204, 384, 408, 384, 1204, 384, 412, 380, 1204, 384, 1204, 384, 1204, 384, 412, 384, 408, 384, 408, 388, 408, 384, 1204, 384, 1204, 380, 412, 384, 1208, 380, 1204, 384, 412, 380, 412, 384, 1204, 384, 408, 384, 412, 384, 1200, 384, 412, 384, 412, 380, 1204, 384, 1204, 384, 412, 384, 1204, 380, 1208, 380, 1204, 384, 412, 384, 1204, 384, 1204, 384, 1204, 380, 1204, 384, 1204, 384, 1204, 384, 412, 384, 1204, 380, 412, 384, 408, 388, 408, 384, 408, 384, 412, 384, 408, 384, 1204, 384, 1204, 384, 1204, 384, 408, 384, 1204, 384, 1204, 384, 1204, 384, 1204, 384, 408, 384, 412, 384, 408, 384, 1204, 384, 412, 384, 408, 384, 412, 384, 408, 384, 408, 384, 1204, 384, 1204, 384, 1204, 384, 412, 380, 1208, 380, 1208, 384, 1200, 384, 1204, 384, 412, 380, 412, 384, 412, 380, 1208, 384, 408, 384, 412, 380, 412, 384      
      };  

    uint16_t ON[179] = {
3204, 1544, 388, 408, 388, 1200, 384, 408, 388, 408, 384, 1200, 440, 356, 388, 1200, 440, 352, 440, 356, 388, 1200, 440, 1144, 388, 1204, 436, 356, 440, 1148, 440, 352, 440, 1148, 440, 1148, 440, 1148, 440, 352, 440, 356, 440, 352, 440, 356, 388, 1200, 436, 1148, 440, 356, 436, 1152, 436, 1152, 436, 356, 440, 356, 436, 1148, 440, 356, 436, 356, 440, 1148, 440, 356, 436, 356, 440, 1148, 440, 1148, 440, 352, 440, 1148, 440, 1148, 436, 1152, 440, 356, 436, 1148, 440, 1148, 440, 1148, 436, 1156, 432, 1152, 436, 1152, 436, 384, 408, 1180, 408, 384, 412, 380, 412, 384, 412, 380, 412, 384, 412, 380, 412, 1176, 412, 1176, 412, 1176, 412, 380, 412, 1176, 412, 1176, 412, 1176, 412, 1176, 412, 380, 412, 384, 412, 380, 412, 1176, 412, 384, 408, 384, 412, 380, 412, 384, 412, 380, 412, 1176, 412, 1176, 412, 384, 412, 380, 412, 1176, 412, 1176, 412, 1176, 412, 1172, 412, 384, 412, 384, 412, 1172, 412, 1176, 412, 384, 412, 380, 412, 384, 412
      };  // UNKNOWN 6592A5A4

      //18C, static, high
     uint16_t COLD[179] = {
3148, 1600, 384, 412, 384, 1204, 384, 408, 384, 412, 380, 1204, 384, 412, 384, 1204, 384, 408, 384, 412, 384, 1204, 384, 1204, 384, 1204, 384, 408, 384, 1204, 384, 408, 384, 1204, 384, 1204, 384, 1204, 384, 408, 384, 412, 384, 408, 384, 412, 384, 1204, 384, 1204, 384, 408, 384, 1204, 384, 1204, 384, 408, 384, 412, 384, 1204, 384, 408, 384, 412, 384, 1200, 388, 408, 384, 412, 380, 1204, 384, 1204, 384, 412, 384, 1204, 384, 1204, 384, 1200, 384, 412, 384, 1204, 384, 1204, 384, 1204, 380, 1204, 384, 1204, 384, 1204, 384, 412, 384, 1204, 384, 408, 384, 412, 384, 408, 384, 412, 380, 412, 384, 412, 380, 1204, 384, 1204, 384, 1204, 384, 1204, 384, 412, 384, 1200, 384, 1204, 384, 412, 384, 408, 384, 412, 384, 408, 384, 412, 384, 1204, 380, 412, 384, 412, 380, 1204, 384, 412, 384, 1204, 384, 1204, 380, 412, 384, 408, 384, 1204, 384, 1204, 384, 1204, 384, 1204, 384, 412, 380, 412, 384, 1204, 384, 1204, 384, 408, 384, 416, 380, 408, 384         
      };

            //22C, moving, auto
      uint16_t ECONO[179] = {
3248, 1544, 440, 352, 440, 1148, 444, 352, 440, 352, 440, 1148, 440, 356, 440, 1148, 440, 352, 416, 376, 444, 1144, 444, 1144, 440, 1148, 440, 356, 440, 1148, 440, 352, 440, 1148, 440, 1148, 440, 1148, 440, 352, 440, 356, 440, 352, 444, 352, 440, 1148, 440, 1144, 444, 352, 440, 1148, 440, 1148, 440, 352, 440, 356, 440, 1148, 440, 352, 440, 356, 440, 1148, 440, 352, 440, 352, 444, 1148, 440, 1144, 444, 352, 440, 1148, 440, 1148, 440, 1148, 440, 1148, 440, 1148, 440, 1144, 444, 1148, 436, 1148, 440, 1148, 440, 1148, 440, 356, 440, 352, 440, 356, 440, 352, 440, 356, 440, 352, 440, 356, 436, 356, 440, 1148, 416, 1172, 440, 1148, 440, 1148, 440, 352, 440, 1148, 440, 1148, 440, 1148, 440, 352, 440, 356, 440, 352, 440, 356, 440, 1148, 440, 352, 440, 356, 440, 352, 440, 352, 444, 1148, 440, 1148, 436, 356, 440, 352, 440, 1148, 440, 356, 436, 1152, 440, 1148, 440, 352, 440, 356, 440, 1144, 440, 1152, 436, 356, 440, 1148, 440, 352, 440 

      };

void setPowerStateOnServer(String deviceId, String value);
void setTargetTemperatureOnServer(String deviceId, String value, String scale);

// deviceId is the ID assgined to your smart-home-device in sinric.com dashboard. Copy it from dashboard and paste it here

void turnOn(String deviceId) {
  if (deviceId == LAMP) // Device ID of first device
  {  
    Serial.print("Turn on device id: LAMP "); 
    isDISCO_ON = false;
    RGB_color(255, 255, 255);
  } else if (deviceId == AIRCORN) {
    Serial.print("Turn on device id: AIRCORN ");
    irsend.sendRaw(ON, 179, 40);  
  } else if (deviceId == DISCO) {
    Serial.print("Turn on device id: DISCO");
    isDISCO_ON = true;
  } else if (deviceId == AC_HIGH) {
    Serial.print("Turn on device id: AC_HIGH");
    irsend.sendRaw(COLD, 179, 40);  
  } else if (deviceId == AC_LOW) {
    Serial.print("Turn on device id: AC_LOW");
    irsend.sendRaw(ECONO, 179, 40);  
  } else {
    Serial.print("Turn on for unknown device id: ");
    Serial.println(deviceId);    
  }     
}

void turnOff(String deviceId) {
   if (deviceId == LAMP){  
     Serial.print("Turn off LAMP");
     RGB_color(LOW, LOW, LOW);
   } else if (deviceId == AIRCORN) {
    Serial.print("Turn off AIRCORN: ");
    irsend.sendRaw(OFF, 179, 40);
   } else if (deviceId == DISCO) {
    Serial.print("Turn off DISCO: ");
    isDISCO_ON = false;
   } else if (deviceId == AC_HIGH) {
    Serial.print("Turn off : AC_HIGH ");
       irsend.sendRaw(OFF, 179, 40);
   } else if (deviceId == AC_LOW) {
    Serial.print("Turn off : AC_LOW ");
       irsend.sendRaw(OFF, 179, 40);
  } else {
     Serial.print("Turn off for unknown device id: ");
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

        // For Switch or Light device types
        // {"deviceId": xxxx, "action": "setPowerState", value: "ON"} // https://developer.amazon.com/docs/device-apis/alexa-powercontroller.html

        // For Light device type
        // Look at the light example in github
          
#if ARDUINOJSON_VERSION_MAJOR == 5
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
        DynamicJsonDocument json(1024);
        deserializeJson(json, (char*) payload);      
#endif        
        String deviceId = json ["deviceId"];     
        String action = json ["action"];
        
        if(action == "setPowerState") { // Switch or Light
            String value = json ["value"];
            if(value == "ON") {
                turnOn(deviceId);
            } else {
                turnOff(deviceId);
            }
        }
        else if (action == "SetTargetTemperature") {
            String deviceId = json ["deviceId"];     
            String action = json ["action"];
            String value = json ["value"];
        }
        else if (action == "SetColor") {
            // {"deviceId": xxxx, "action": "SetColor", value: {"hue": 350.5,  "saturation": 0.7138, "brightness": 0.6501}} // https://developer.amazon.com/docs/device-apis/alexa-colorcontroller.html
            String hue = json ["value"]["hue"];
            String saturation = json ["value"]["saturation"];
            String brightness = json ["value"]["brightness"];
            isDISCO_ON = false;
            
            H2R_HSBtoRGB(hue.toFloat(), saturation.toFloat() * 100, brightness.toFloat() * 100, rgb_arr);
            Serial.println("HSB:" + hue + " " + saturation + " " + brightness);
            Serial.println(rgb_arr[0]);
            Serial.println(rgb_arr[1]);
            Serial.println(rgb_arr[2]);
            RGB_color(rgb_arr[0], rgb_arr[1], rgb_arr[2]);
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
  Serial.begin(115200);
  irsend.begin();
  // Relay PIN eg: https://github.com/wemos/D1_mini_Examples/blob/master/examples/04.Shields/Relay_Shield/Blink/Blink.ino
  pinMode(relayPin, OUTPUT);
  pinMode(red_light_pin, OUTPUT);
  pinMode(green_light_pin, OUTPUT);
  pinMode(blue_light_pin, OUTPUT);
  
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
  webSocket.begin("iot.sinric.com", 80, "/");

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
 
    if(isDISCO_ON) {
      RGB_color(255, 0, 0); // Red
      delay(750);
      RGB_color(0, 255, 0); // Green
      delay(750);
      RGB_color(0, 0, 255); // Blue
      delay(750);
      RGB_color(255, 255, 125); // Raspberry
      delay(750);
      RGB_color(0, 255, 255); // Cyan
      delay(750);
      RGB_color(255, 0, 255); // Magenta
      delay(750);
      RGB_color(255, 255, 0); // Yellow
      delay(750);
      RGB_color(255, 255, 255); // White
      delay(750);
    }
}
     
}

// If you are going to use a push button to on/off the switch manually, use this function to update the status on the server
// so it will reflect on Alexa app.
// eg: setPowerStateOnServer("deviceid", "ON")

// Call ONLY If status changed. DO NOT CALL THIS IN loop() and overload the server. 
void setPowerStateOnServer(String deviceId, String value) {
#if ARDUINOJSON_VERSION_MAJOR == 5
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
  DynamicJsonDocument root(1024);
#endif        
  root["deviceId"] = deviceId;
  root["action"] = "setPowerState";
  root["value"] = value;
  StreamString databuf;
#if ARDUINOJSON_VERSION_MAJOR == 5
  root.printTo(databuf);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
  serializeJson(root, databuf);
#endif  
  
  webSocket.sendTXT(databuf);
}

//eg: setPowerStateOnServer("deviceid", "CELSIUS", "25.0")

// Call ONLY If status changed. DO NOT CALL THIS IN loop() and overload the server. 
void setTargetTemperatureOnServer(String deviceId, String value, String scale) {
#if ARDUINOJSON_VERSION_MAJOR == 5
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
  DynamicJsonDocument root(1024);
#endif        
  root["action"] = "SetTargetTemperature";
  root["deviceId"] = deviceId;

#if ARDUINOJSON_VERSION_MAJOR == 5
  JsonObject& valueObj = root.createNestedObject("value");
  JsonObject& targetSetpoint = valueObj.createNestedObject("targetSetpoint");
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
  JsonObject valueObj = root.createNestedObject("value");
  JsonObject targetSetpoint = valueObj.createNestedObject("targetSetpoint");
#endif  
  targetSetpoint["value"] = value;
  targetSetpoint["scale"] = scale;
   
  StreamString databuf;
#if ARDUINOJSON_VERSION_MAJOR == 5
  root.printTo(databuf);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
  serializeJson(root, databuf);
#endif  
  
  webSocket.sendTXT(databuf);
}


void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(red_light_pin, red_light_value);
  analogWrite(green_light_pin, green_light_value);
  analogWrite(blue_light_pin, blue_light_value);
}
