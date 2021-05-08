/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 Arduino module, will be 
 *  printed to Serial when the module is connected.
 */

#include <ESP8266WiFi.h>
#include <EEPROM.h>
const char* WIFI_SSID = "Lyncsis";
const char* WIFI_PASSWORD = "xxxxxxxxxxxxx";
const int LED_PIN = 2;
const int GPIO_PIN_0 = 0;


int currentValue = 0;
String deviceName = "";
// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  EEPROM.begin(512);
  Serial.begin(115200);
  delay(10);

  deviceName = loadDeviceName();
  
  
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
  WiFi.hostname(deviceName);
  Serial.println(deviceName);
}

void loop() {

   while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      blink(3,250);
      Serial.print("wifi disconnected...");
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

  Serial.print(req);
      
  if (req.indexOf("/gpio/0") != -1)
  { 
      // turn pin off
      currentValue = 0;
  }
  else if (req.indexOf("/gpio/1") != -1)
  {
      // turn pin on
      currentValue = 1;
  }
  else if (req.indexOf("/settings") != -1)
  {
      // set the device name
      String newName = getQueryStringValue(req,"name");
      saveDeviceName(newName);
      deviceName = loadDeviceName();
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
  String("<a href=\"/gpio/" + String(currentValue == 0 ? 1 : 0) + "\" class=\"button\">") + deviceName + String("</a>") + 
  String("</div>") + 
  String("</html>");
  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disconnected");
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

String getQueryStringValue(String line, String needle)
{
  // GET /settings/name?name=test HTTP/1.1 
  String request = getValue(line, ' ', 1);
  Serial.println(request);

  // /settings/name?A=test&B=something
  String queryString = getValue(request, '?', 1);
  Serial.println(queryString);

  // A=test&B=something
  for(int i=0; i < 10; i++)
  {
    String parameter = getValue(queryString, '&', i);
    Serial.println(parameter );
    if(parameter != "")
    {
      String key = getValue(parameter,'=',0);
      String value = getValue(parameter,'=',1);
         Serial.println(key + ":" + value);
      if(key == needle)
      {
        return value;
      }
    }
  }

  return "";
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void saveDeviceName(String name)
{
   writeEEPROMString(0, name);
}

String loadDeviceName()
{
  String deviceNameFromRom = readEEPROMString(0);
  
  Serial.println(deviceNameFromRom);

  if(deviceNameFromRom.length() == 0)
  {
    return "ESP8266";
  }
  else
  {
    Serial.println(deviceNameFromRom);
    return deviceNameFromRom;
  }
}


void writeEEPROMString(char add, String data)
{
  int _size = data.length();
  int i;
  for(i=0;i<_size;i++)
  {
    EEPROM.write(add+i,data[i]);
  }
  EEPROM.write(add+_size,'\0');   //Add termination null character for String Data
  EEPROM.commit();
}


/* This will read whatever is stored in the first 100 bytes and return everything be fore the null terminator.
 * If we want to store other values we would need to pass an 'add' value to ssay where that value starts. 
 */
String readEEPROMString(char add)
{
  int i;
  char data[100]; //Max 100 Bytes
  int len=0;
  unsigned char k;
  k=EEPROM.read(add);
  while(k != '\0' && len<100)   //Read until null character
  {    
    k=EEPROM.read(add+len);
    data[len]=k;
    len++;
  }
  data[len]='\0';
  return String(data);
}
