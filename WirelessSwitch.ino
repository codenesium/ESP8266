/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 Arduino module, will be 
 *  printed to Serial when the module is connected.
 */

#include <ESP8266WiFi.h>

const char* DEVICE_NAME = "ESP8266-01";
const char* WIFI_SSID = "Lyncsis";
const char* WIFI_PASSWORD = "xxxxxxxxxxxxx";
const int LED_PIN = 2;
const int GPIO_PIN_0 = 0;


int currentValue = 0;
// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  // setup GPIO and LED
  pinMode(GPIO_PIN_0, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(GPIO_PIN_0, 1);

  // blink the LED so we know it's working
  blink(5,250);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
  WiFi.hostname(DEVICE_NAME);
  Serial.println(DEVICE_NAME);
}

void loop() {

   if (WiFi.status() != WL_CONNECTED) {
      delay(500);
      blink(3,250);
      Serial.print("wifi disconnected...");
      return;
    }
    
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  if (req.indexOf("/gpio/0") != -1)
  { 
      currentValue = 0;
  }
  else if (req.indexOf("/gpio/1") != -1)
  {
      currentValue = 1;
  }

  digitalWrite(LED_PIN, currentValue == 0 ? 1 : 0);
  digitalWrite(GPIO_PIN_0, currentValue == 0 ? 1 : 0);
  
  client.flush();

  // Prepare the response
  String s = String("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>") + 
  String("<style>html {height: 100%;}body {min-height: 100%;}</style>") +
  String("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">") + 
  String("<style>.button {width:100%;height:100%;background-color:") + String(currentValue == 0 ? "Grey;" : "Green;") + String("border: none; color: white;text-align: center;text-decoration: none;display: inline-block;font-size: 64px;}</style>") + 
  String("<div style=\"height:285px; margin:10px; line-height:285px;text-align:center;\">") + 
  String("<a href=\"/gpio/" + String(currentValue == 0 ? 1 : 0) + "\" class=\"button\">") + DEVICE_NAME + String("</a>") + 
  String("</div>") + 
  String("</html>");
  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disconnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

void blink(int count, int durationInMS)
{
  for(int i=0; i < count;i++)
  {
    digitalWrite(LED_PIN, 0);
    delay(durationInMS);
    digitalWrite(LED_PIN, 1);
    delay(durationInMS);
  }
}
