#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WifiManager.h>
#include <ArduinoJson.h>

#define DEBUG 0  //Print to Serial

const short int BUILTIN_LED1 = 1; //GPIO1 LOW is on
int BUTTON = 2;

#define PRINTDEBUG(STR) \
  {  \
    if (DEBUG) Serial.println(STR); \
  }
#define BIAS 1

//INIT
const char* WIFI_CONFIG_SSID  = "Hogwarts";
const char* WIFI_CONFIG_PASSWORD  = "xxxxxxxxxxxxx";
const char* HOST = "192.168.0.95";
const char* DEVICE_ID = "ESP8266-01";
const int PORT = 8080;
const int BLINK_DURATION_STATUS = 100;
const int BLINK_DURATION_ERROR = 1000;
const String BLINK_ACTION = "Blink";

// Inifinite loop - Causes to reset self
void resetSelf() {
  PRINTDEBUG("Reseting");
  while (1) {}
}



void blinkLED(int times,int durationInMS) {
  for(int i=0; i < times; i++) {
    
    digitalWrite(BUILTIN_LED1, LOW);
    delay(durationInMS);
    digitalWrite(BUILTIN_LED1, HIGH);
    delay(durationInMS);
  }
}

void blinkError() {
  blinkLED(3,BLINK_DURATION_ERROR);
}

// Setup
void setup() {
  
  if (DEBUG){ 
    Serial.begin(115200);  //Start Serial
  } 
  else {
    pinMode(BUILTIN_LED1,OUTPUT);
    digitalWrite(BUILTIN_LED1, HIGH);
  }

  pinMode(BUTTON,INPUT);
  delay(10);
  connectWifi(WIFI_CONFIG_SSID,WIFI_CONFIG_PASSWORD);

  //sendRequest(HOST, PORT);
  //PRINTDEBUG("Sleeping ");
  //ESP.deepSleep(10000000);
 
}




////////////////////////// LOOP //////////////////////////////////////////////////////
void loop() {

 int buttonState = digitalRead(BUTTON);

 if(buttonState == 1)
 {
    sendRequest(HOST, PORT);
 }
 
 delay(100);
}

// Function to connect WiFi
void connectWifi(const char* ssid, const char* password) {
  int WiFiCounter = 0;
  // We start by connecting to a WiFi network
  PRINTDEBUG("Connecting to ");
  PRINTDEBUG(ssid);
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  if(WiFi.SSID() != ssid) {
    WiFi.begin(ssid, password);
  }
  
  while (WiFi.status() != WL_CONNECTED && WiFiCounter < 30) {
    delay(1000);
    WiFiCounter++;
    PRINTDEBUG(".");
  }

  PRINTDEBUG("");
  PRINTDEBUG("WiFi connected");
  PRINTDEBUG("IP address: ");
  PRINTDEBUG(WiFi.localIP());
}

String sendRequest(String host, int port) {
  String url = "/deviceId=" + String(DEVICE_ID) + "&action=button_pressed";
  String request = String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + HOST + "\r\n" +
               "Connection: close\r\n\r\n";

  String response = httpRequest(HOST,PORT,request);

  return response;
}

String httpRequest(String host, int port, String request) {


  PRINTDEBUG("connecting to ");
  PRINTDEBUG(host); 
  WiFiClient client;
  
  if (!client.connect(host, port)) {
    PRINTDEBUG("connection failed");
    blinkError();
    return "error";
  } 
  else {
    PRINTDEBUG("connection established");
  }

  // This will send the request to the server

  PRINTDEBUG("Request: " + request);
  client.print(request);
  
  int dataFlagCounter = 0;
  int whileTimeout = 0;
  whileTimeout = millis();

  String line;
  /*
  while(client.connected()) {
    while (client.available()) {
        line = client.readStringUntil('\r'); // This should return the last line of the response.
                                             // We read all of the header and just get the data.
    }
  }
  */

  client.stop();
  delay(250);
  PRINTDEBUG("closing connection");
  
  PRINTDEBUG("line:" + line); // will be the last line of the response. 
  return line;

}


 
